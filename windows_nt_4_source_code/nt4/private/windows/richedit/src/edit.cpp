/*
 *	@doc INTERNAL
 *
 *	@module	EDIT.C - main part of CTxtEdit |
 *	
 *		See also textserv.cpp (ITextServices and SendMessage interfaces)
 *		and tomDoc.cpp (ITextDocument interface)
 *	
 *	Authors: <nl>
 *		Original RichEdit code: David R. Fulmer <nl>
 *		Christian Fortini, Murray Sargent, Alex Gounares, Rick Sailor,
 *		Jon Matousek
 *	
 *	History: <nl>
 *		12/28/95 jonmat-Added support of Magellan mouse and smooth scrolling.
 *
 *	@devnote
 *		Be sure to set tabs at every four (4) columns.  In fact, don't even
 *		think of doing anything else!
 *
 *	Copyright (c) 1995-1996 Microsoft Corporation. All rights reserved.
 */

#include "_common.h"
#include "_edit.h"
#include "_dispprt.h"
#include "_dispml.h"
#include "_dispsl.h"
#include "_select.h"
#include "_text.h"
#include "_runptr.h"
#include "_font.h"
#include "_measure.h"
#include "_render.h"
#include "_m_undo.h"
#include "_antievt.h"
#include "_rtext.h"

#include "_ime.h"
#include "_NLSPRCS.h"
#include "_urlsup.h"


#define CTRL(_ch) (_ch - 'A' + 1)


ASSERTDATA

#ifdef DEBUG
BOOL fInAssert = FALSE;
#endif

// This is not public because we don't really want folks using it.
// ITextServices is a private interface.
EXTERN_C const IID IID_ITextServices = { // 8d33f740-cf58-11ce-a89d-00aa006cadc5
	0x8d33f740,
	0xcf58,
	0x11ce,
	{0xa8, 0x9d, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5}
  };

// {13E670F4-1A5A-11cf-ABEB-00AA00B65EA1}
EXTERN_C const GUID IID_ITextHost = 
{ 0x13e670f4, 0x1a5a, 0x11cf, { 0xab, 0xeb, 0x0, 0xaa, 0x0, 0xb6, 0x5e, 0xa1 } };

// {13E670F5-1A5A-11cf-ABEB-00AA00B65EA1}
EXTERN_C const GUID IID_ITextHost2 = 
{ 0x13e670f5, 0x1a5a, 0x11cf, { 0xab, 0xeb, 0x0, 0xaa, 0x0, 0xb6, 0x5e, 0xa1 } };

// Static data members

DWORD CTxtEdit::_dwTickDblClick;	// time of last double-click
POINT CTxtEdit::_ptDblClick;		// position of last double-click

HCURSOR CTxtEdit::_hcurArrow = 0;
HCURSOR CTxtEdit::_hcurSelBar = 0;
#ifndef DBCS
HCURSOR CTxtEdit::_hcurIBeam = 0;
#endif

// ??? CF - These should probably go away
#pragma BEGIN_CODESPACE_DATA
#ifdef MACPORT
TCHAR szCRLF[]	= TEXT("\n");
TCHAR szCR[]	= TEXT("\n");
#else
TCHAR szCRLF[]	= TEXT("\r\n");
TCHAR szCR[]	= TEXT("\r");
#endif
#pragma END_CODESPACE_DATA

#ifdef PENWIN20
// hit test codes for CTxtEdit::OnHitTest
#define lHitText	1	// hit text
#define lHitSel		2	// hit selection
#endif // PENWIN20

LOCAL WORD 		ConvVKey (WORD vKey);



///////////////// CTxtEdit Creation, Initialization, Destruction ///////////////////////////////////////

/*
 *	CTxtEdit::CTxtEdit()
 *
 *	@mfunc
 *		constructor
 */
CTxtEdit::CTxtEdit(ITextHost *phost, IUnknown *punk)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::CTxtEdit");

	_unk.Init();
	_punk = (punk) ? punk : &_unk;
	_ldte.Init(this);
	_phost	  = phost;
	_cpAccelerator = -1;					// Default to no accelerator

	//initialize _iCF and _iPF to something bogus
	_iCF = -1;
	_iPF = -1;

	// Initialize local maximum text size to window default
	_cchTextMost = cInitTextMax;
}

/*
 *	CTxtEdit::~CTxtEdit()
 *
 *	@mfunc
 *		Destructor
 */
CTxtEdit::~CTxtEdit ()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::~CTxtEdit");

	Assert(!_fMButtonCapture);				// Need to properly transition
											//  Magellan mouse if asserts!

	_fSelfDestruct = TRUE;					// Tell the Call Mgr not to
											//  call this any more
	if( _pdetecturl )
	{
		delete _pdetecturl;
	}

	if(_pDocInfo)							// Do this before closing
	{										//  down internal structures
		CloseFile(TRUE);					// Close any open file
		delete _pDocInfo;					// Delete document info
	}

	ReleaseFormats(_iCF, _iPF);				// Release the default formats

	// Flush the clipboard first

	_ldte.FlushClipboard();

	if( _pobjmgr )
	{
		delete _pobjmgr;
	}

	// Release our reference to selection object
	if (_psel)
	{
		_psel->Release();
	}

	// Delete undo manager
	if( _pundo )
	{
		_pundo->Destroy();
	}
	if( _predo )
	{
		_predo->Destroy();
	}

	delete _ime;					// Delete IME
	delete _pdp;					// Delete displays
	delete _pdpPrinter;
	_pdp = NULL;					// Break any further attempts to use
}									//  display

/*
 *	CTxtEdit::Init (prcClient)
 *
 *	@mfunc
 *		Initializes this CTxtEdit. Called by CreateTextServices()
 *
 *	@rdesc
 *		Return TRUE if successful
 */
BOOL CTxtEdit::Init (
	const RECT *prcClient)		//@parm Client RECT
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::Init");

	CCharFormat 		CF;
	DWORD				dwBits = 0;
	LONG				iCF, iPF;
	CParaFormat 		PF;
	ICharFormatCache *	pICFCache;
	IParaFormatCache *	pIPFCache;
	CCallMgr			callmgr(this);

	// Set up default CCharFormat and CParaFormat

	if( TxGetDefaultCharFormat(&CF) != NOERROR ||
		TxGetDefaultParaFormat(&PF) != NOERROR )
	{
		return FALSE;
	}
													// Got default formats
	if (FAILED(GetCharFormatCache(&pICFCache)) ||	// Now try to cache them
		FAILED(GetParaFormatCache(&pIPFCache)) ||
		FAILED(pICFCache->Cache(&CF, &iCF))	   ||
		FAILED(pIPFCache->Cache(&PF, &iPF)))
	{
		return FALSE;
	}												// Cached default formats
	_iCF = (WORD)iCF;								// Save format indices
	_iPF = (WORD)iPF;

	// Load mouse cursors (but only for first instance)
	if(!_hcurArrow)
	{
		_hcurArrow = LoadCursor(0, IDC_ARROW);

		if(!_hcurIBeam)								// Load cursor
			_hcurIBeam = LoadCursor(0, IDC_IBEAM);

		if(!_hcurSelBar)
			_hcurSelBar = LoadCursor(hinstRE, MAKEINTRESOURCE(CUR_SELBAR));
	}

	_pfnWB = (EDITWORDBREAKPROC) TxWordBreakProc;	// Set word breaking proc

#ifdef DEBUG
	// the host is going to do some checking on richtext vs. plain
	// text.
	_fRich = TRUE;
#endif // DEBUG

	if(_phost->TxGetPropertyBits (TXTBITS |		// Get host state flags
		TXTBIT_MULTILINE | TXTBIT_SHOWACCELERATOR,	//  that we cache or need
		&dwBits) != NOERROR )						//  for display setup
	{
		return FALSE;
	}												// Cache bits defined by
	_dwFlags = dwBits & TXTBITS;					//  TXTBITS mask

	if ((dwBits & TXTBIT_SHOWACCELERATOR) &&		// They want accelerator,
		FAILED(UpdateAccelerator()))				//  so let's get it
	{
		return FALSE;
	}		

	_fTransparent = TxGetBackStyle() == TXTBACK_TRANSPARENT;
	if( dwBits & TXTBIT_MULTILINE )					// Create and initialize
		_pdp = new CDisplayML(this);				//  display
	else
		_pdp = new CDisplaySL(this);

	if(!_pdp || !_pdp->Init())
		return FALSE;

	_fUseUndo = TRUE;

 	// Initialize some IME bits
	_fAutoFont = TRUE;
	_fAutoKeyboard = FALSE;


	// Set whether we are in our host or not
	ITextHost2 *phost2;
	if( _phost->QueryInterface(IID_ITextHost2, (void **)&phost2) 
		== NOERROR )
	{
		// We assume that ITextHost2 means this is our host
		phost2->Release();
		_fInOurHost = TRUE;
	}
	else
	{
		// Get the maximum from our host
		_phost->TxGetMaxLength((DWORD *) &_cchTextMost);
	}
	
	//			
	// Add EOP iff Rich Text
	//
	if (IsRich())
	{
		// we should _not_ be in 10 compatibility mode yet.
		// if we transition into 1.0 mode, we'll add a CRLF
		// at the end of the document.
		SetRichDocEndEOP(0);
	}

	// for Japanese, this virtual key indicates Kana mode.  
	// Check the toggle bit to see if it is on or offf.
	// If on, Kana characters (single byte Japanese char) are coming in via WM_CHAR.
	_fKANAMode = (GetKeyboardCodePage() == _JAPAN_CP) &&
		((GetKeyState(VK_KANA) & 0x1) != 0);

	return TRUE;
}


///////////////////////////// CTxtEdit IUnknown ////////////////////////////////

/*
 *	CTxtEdit::QueryInterface (riid, ppv)
 *
 *	@mfunc
 *		IUnknown method
 *
 *	@rdesc
 *		HRESULT = (if success) ? NOERROR : E_NOINTERFACE
 *
 *	@devnote
 *		This interface is aggregated. See textserv.cpp for discussion.
 */
HRESULT CTxtEdit::QueryInterface(REFIID riid, void **ppv)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::QueryInterface");

	return _punk->QueryInterface(riid, ppv);
}

/*
 *	CTxtEdit::AddRef()
 *
 *	@mfunc
 *		IUnknown method
 *
 *	@rdesc
 *		ULONG - incremented reference count
 */
ULONG CTxtEdit::AddRef(void)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::AddRef");

	return _punk->AddRef();
}

/*
 *	CTxtEdit::Release()
 *
 *	@mfunc
 *		IUnknown method
 *
 *	@rdesc
 *		ULONG - decremented reference count
 */
ULONG CTxtEdit::Release(void)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::Release");

	return _punk->Release();
}

////////////////////////// Undo Management  //////////////////////////////

/*
 *	CTxtEdit::CreateUndoMgr (dwLim, flags)
 *
 *	@mfunc
 *		Creates an undo stack
 *
 *	@rdesc
 *		Ptr to new IUndoMgr 
 */
IUndoMgr *CTxtEdit::CreateUndoMgr(
	DWORD	dwLim,			//@parm Size limit for the
	USFlags flags)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::CreateUndoMgr");

	IUndoMgr *pmgr = NULL;

	if( _fUseUndo )
	{
		pmgr = new CUndoStack(this, dwLim, flags);
		if( pmgr == NULL || pmgr->GetUndoLimit() == 0 )
		{
			// The undo stack failed to initialize properly (probably
			// lack of memory). Trash it and return NULL.
			if( pmgr != NULL)
			{
				pmgr->Destroy();
			}
			return NULL;
		}
		if( flags & US_REDO )
		{
			_predo = pmgr;
		}
		else
		{
			_pundo = pmgr;
		}
	}
	
	return pmgr;
}

/*
 *	CTxtEdit::HandleUndoLimit (dwLim)
 *
 *	@mfunc
 *		Handles the EM_SETUNDOLIMIT message
 *
 *	@rdesc	
 *		Actual limit to which things were set.
 */
LRESULT CTxtEdit::HandleSetUndoLimit( 
	DWORD dwLim ) 		//@parm	Requested limit size
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::HandleSetUndoLimit");

	if( dwLim == 0 )
	{
		_fUseUndo = FALSE;

		if( _pundo )
		{
			_pundo->Destroy();
			_pundo = NULL;
		}

		if( _predo )
		{
			_predo->Destroy();
			_predo = NULL;
		}
	}
	else if ( !_pundo )
	{
		_fUseUndo = TRUE;
		// don't worry about return value; if it's NULL, we're
		// in the same boat as if the api wasn't called ( so later
		// on, we might try to allocate the default).
		CreateUndoMgr(dwLim, US_UNDO);
	}
	else
	{
		dwLim = _pundo->SetUndoLimit(dwLim);

		// setting the undo limit on the undo stack will return to
		// us the actual amount set.  Try to set the redo stack to 
		// the same size.  If it can't go that big, too bad.

		if( _predo )
		{
			_predo->SetUndoLimit(dwLim);
		}
	}
	return dwLim;
}

/*
 *	CTxtEdit::HandleSetTextMode
 *
 *	@mfunc	handles setting the text mode
 *
 *	@rdesc	LRESULT; 0 (NOERROR) on success, OLE failure code on failure.
 *
 *	@devnote	the text mode does not have to be fully specified; it
 *			is sufficient to merely specify the specific desired behavior.
 *
 *			Note that the edit control must be completely empty for this
 *			routine to work.
 */
LRESULT CTxtEdit::HandleSetTextMode(
	DWORD mode)			//@parm the desired mode
{
	LRESULT lres = 0;

	// first off, we must be completely empty
	if( GetAdjustedTextLength() || 
		(_pundo && _pundo->CanUndo()) ||
		(_predo && _predo->CanUndo()) )
	{
		return E_UNEXPECTED;
	}

	// These bits are considered one at a time; thus the absence of
	// any bits does _NOT_ imply any change in behavior.

	// TM_RICHTEXT && TM_PLAINTEXT are mutually exclusive; they cannot
	// be both set.  Same goes for TM_SINGLELEVELUNDO / TM_MULTILEVELUNDO
	// and TM_SINGLECODEPAGE / TM_MULTICODEPAGE
	if( ((mode & (TM_RICHTEXT | TM_PLAINTEXT)) == 
			(TM_RICHTEXT | TM_PLAINTEXT)) ||
		((mode & (TM_SINGLELEVELUNDO | TM_MULTILEVELUNDO)) ==
			(TM_SINGLELEVELUNDO | TM_MULTILEVELUNDO) ) ||
		((mode & (TM_SINGLECODEPAGE | TM_MULTICODEPAGE)) ==
			(TM_SINGLECODEPAGE | TM_MULTICODEPAGE)) )
	{
		lres = E_INVALIDARG;
	}
	else if( (mode & TM_PLAINTEXT) && IsRich())
	{
		lres = OnRichEditChange(FALSE);
	}
	else if( (mode & TM_RICHTEXT) && !IsRich())
	{
		lres = OnRichEditChange(TRUE);
	}

	if( lres == 0 )
	{
		if( (mode & TM_SINGLELEVELUNDO) )
		{
			if( !_pundo )
			{
				CreateUndoMgr(1, US_UNDO);
			}

			if( _pundo )
			{
				// we can 'Enable' single level mode as many times
				// as we want, so no need to check for it before hand.
				lres = ((CUndoStack *)_pundo)->EnableSingleLevelMode();
			}
			else
			{
				lres = E_OUTOFMEMORY;
			}
		}
		else if( (mode & TM_MULTILEVELUNDO) )
		{
			// if there's no undo stack, no need to do anything,
			// we're already in multi-level mode
			if( _pundo && ((CUndoStack *)_pundo)->GetSingleLevelMode())
			{
				((CUndoStack *)_pundo)->DisableSingleLevelMode();
			}
		}

		if( (mode & TM_SINGLECODEPAGE) )
		{
			_fSingleCodePage = TRUE;
		}
		else if( (mode & TM_MULTICODEPAGE) )
		{
			_fSingleCodePage = FALSE;
		}
	}

	// We don't want this marked modified after this operation to make us
	// work better in dialog boxes.
	_fModified = FALSE;

	return lres;
}

////////////////////////// Notification Manager //////////////////////////////

/*
 *	CTxtEdit::GetNotifyMgr()
 *
 *	@mfunc
 *		returns a pointer to the notification manager (creating it if necessary)
 *
 *	@rdesc
 *		Ptr to notification manager
 */
CNotifyMgr *CTxtEdit::GetNotifyMgr()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::GetNotifyMgr");

	return &_nm;
}


////////////////////////// Object Manager ///////////////////////////////////

/*
 *	CTxtEdit::GetObjectMgr()
 *
 *	@mfunc
 *		returns a pointer to the object manager (creating if necessary )
 *
 *	@rdesc
 *		pointer to the object manager
 */
CObjectMgr *CTxtEdit::GetObjectMgr()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::GetObjectMgr");

	if( !_pobjmgr )
	{
		_pobjmgr = new CObjectMgr();
	}

	return _pobjmgr;
}


////////////////////////////// Properties - Selection ////////////////////////////////


LONG CTxtEdit::GetSelMin() const
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::GetSelMin");

	LONG cpMin, cpMost;

	if (_psel)
	{
		_psel->GetRange(cpMin, cpMost);
		return cpMin;
	}
	return 0;
}

LONG CTxtEdit::GetSelMost() const
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::GetSelMost");

	LONG cpMin, cpMost;

	if (_psel)
	{
		_psel->GetRange(cpMin, cpMost);
		return cpMost;
	}
	return 0;
}

		
////////////////////////////// Properties - Text //////////////////////////////////////


LONG CTxtEdit::GetTextRange(LONG cpFirst, LONG cch, TCHAR *pch)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::GetTextRange");

#ifdef DEBUG
	const LONG cchAsk = cch;
#endif
	CTxtPtr	tp(this, cpFirst);
	LONG	length = (LONG)GetAdjustedTextLength();

	if(--cch < 0 || cpFirst > length)
		return 0;

	cch = min(cch, length - cpFirst);

	if(cch > 0)
	{
		cch = tp.GetText(cch, pch);
		Assert(cch >= 0);
	}
	pch[cch] = TEXT('\0');

#ifdef DEBUG
	if(cch != cchAsk - 1)
		Tracef(TRCSEVINFO, "CmdGetText(): only got %ld out of %ld", cch, cchAsk - 1);
#endif

	return cch;
}

/*
 *	CTxtEdit::GetTextEx
 *
 *	@mfunc	grabs text according to various params
 *
 *	@rdesc	the number of bytes written
 */
LONG CTxtEdit::GetTextEx(
	GETTEXTEX *pgt,		//@parm info on what to get
	TCHAR *pch)			//@parm where to put the text
{
	CTxtPtr tp(this, 0);
	CTempWcharBuf twcb;
	TCHAR *pchUse = pch;
	LONG cch, cb;
	LONG cchGet = GetAdjustedTextLength();


	// allocate a big buffer; make sure that we have
	// enough room for lots of CRLFs if necessary
	if( (pgt->flags & GT_USECRLF) )
	{
		cchGet *= 2;
	}

	if( pgt->codepage != 1200 )
	{
		// if UNICODE, copy straight to clients buffer;
		// else, copy to temp buffer and translate cases first
		pchUse = twcb.GetBuf(cchGet + 1);
	}
	else
	{
		// be sure to leave room for the NULL terminator.
		cchGet = min(((pgt->cb/2) -1), (DWORD)cchGet);
	}

	// now grab the text.  
	
	if( (pgt->flags & GT_USECRLF) )
	{
		cch = tp.GetPlainText(cchGet, pchUse, tomForward, FALSE);
	}
	else
	{
		cch = tp.GetText(cchGet, pchUse);
	}

	pchUse[cch] = L'\0';

	// if we're just doing UNICODE, return the number of chars written.
	if( pgt->codepage == 1200 )
	{
		return cch;
	}

	// oops, gotta translate to ANSI.

	cb = WideCharToMultiByte(pgt->codepage, 0, pchUse, cch + 1, (char *)pch, 
			pgt->cb, pgt->lpDefaultChar, pgt->lpUsedDefChar);

	// don't count the NULL terminator for compatibility with WM_GETTEXT.
	return (cb) ? cb - 1 : 0;
}
			
/*
 *	CTxtEdit::GetTextLengthEx
 *
 *	@mfunc	calculates the length of the text in various ways.
 *
 *	@rdesc	varies by the input parameter
 */
LONG CTxtEdit::GetTextLengthEx(
	GETTEXTLENGTHEX *pgtl)	//@parm info describing how to calculate the
							// length
{
	LONG cchUnicode = GetAdjustedTextLength();
	GETTEXTEX gt;

	// make sure the flags are definied appropriately.
	if( ((pgtl->flags & GTL_CLOSE) && (pgtl->flags & GTL_PRECISE)) ||
		((pgtl->flags & GTL_NUMCHARS) && (pgtl->flags & GTL_NUMBYTES)) )
	{
		TRACEWARNSZ("Invalid flags for EM_GETTEXTLENGTHEX");
		return E_INVALIDARG;
	}

	if( (pgtl->flags & GTL_USECRLF) )
	{
		CTxtPtr tp(this, 0);
		DWORD numEOP = 0;
		
		while( tp.FindEOP(tomForward) )
		{
			numEOP++;
		}

		// take of the default EOD, if it exists.  Note
		// that in plain text, we may _think_ that there is
		// an EOP at the end when there really isn't.
		if( IsRich() || (numEOP && !tp.IsAfterEOP()))
		{
			Assert(numEOP);
			numEOP--;
		}

		cchUnicode += numEOP;
	}

	// if we're just looking for the number of characters, we've got it.
	if( (pgtl->flags & GTL_NUMCHARS) || !pgtl->flags )
	{
		return cchUnicode;
	}

	// hmm, they're looking for number of bytes, but don't care about 
	// precision, just multiply by two.  If neither PRECISE or CLOSE is
	// specified, default to CLOSE.
	// Note if we're UNICODE and asking for number of bytes, we also
	// just multiply by 2.
	if( (pgtl->flags & GTL_CLOSE) || !(pgtl->flags & GTL_PRECISE) ||
		pgtl->codepage == 1200 )
	{
		return cchUnicode *2;
	}

	// in order to get a precise answer, we're going to need to convert.
	gt.cb = 0;
	gt.flags = (pgtl->flags & GT_USECRLF);
	gt.codepage = pgtl->codepage;
	gt.lpDefaultChar = NULL;
	gt.lpUsedDefChar = NULL;

	return GetTextEx(&gt, NULL);
}


//////////////////////////////  Properties - Formats  //////////////////////////////////

const CCharFormat* CTxtEdit::GetCharFormat(LONG iCF)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::GetCharFormat");

	const CCharFormat *	pCF;
	ICharFormatCache  *	pICache;
	
	if(iCF < 0)
		iCF = _iCF;

	Assert(iCF >= 0);

	if (FAILED(GetCharFormatCache(&pICache)) ||
		FAILED(pICache->Deref(iCF, &pCF)))
	{
		AssertSz(FALSE, "CTxtEdit::GetCharFormat: couldn't deref iCF");
		pCF = NULL;
	}
	return pCF;
}

const CParaFormat* CTxtEdit::GetParaFormat(LONG iPF)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::GetParaFormat");

	IParaFormatCache  *	pICache;
	const CParaFormat *	pPF;
	
	if(iPF < 0)
		iPF = _iPF;

	Assert(iPF >= 0);

	if (FAILED(GetParaFormatCache(&pICache)) ||
		FAILED(pICache->Deref(iPF, &pPF)))
	{
		AssertSz(FALSE, "CTxtEdit::GetParaFormat: couldn't deref iPF");
		pPF = NULL;
	}
	return pPF;
}


//////////////////////////// Mouse Commands /////////////////////////////////


HRESULT CTxtEdit::OnTxLButtonDblClk(INT x, INT y, DWORD dwFlags)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnTxLButtonDblClk");

	BOOL			fEnterParaSelMode = FALSE;
	CTxtSelection *	psel;
	const POINT		pt = {x, y};
	RECT			rc;

	_dwTickDblClick = GetTickCount();
	_ptDblClick.x = x;
	_ptDblClick.y = y;

	TxUpdateWindow();		// Repaint window to show any exposed portions

	if(!_fFocus)
	{
		TxSetFocus();					// Create and display caret
		goto Cleanup;
	}

	// If click isn't inside view, just activate, don't select
	_pdp->GetViewRect(rc);

	if(!PtInRect(&rc, pt))
	{
		if(TxGetSelectionBar())			// Test for double click in selection
		{								//  bar (paragraph selection)
			rc.right = rc.left;
			rc.left = 0;
			if(!PtInRect(&rc, pt))
				return S_OK;
			fEnterParaSelMode = TRUE;
		}
		else
		{
			Assert(_fFocus);
			goto Cleanup;
		}
	}

	_fWantDrag = FALSE;					// just to be safe

	// if we are over a link, let the client have a chance to process
	// the message
	if( HandleLinkNotification(WM_LBUTTONDBLCLK, (WPARAM)dwFlags, 
			MAKELPARAM(x, y)) )
	{
		goto Cleanup;
	}

	if(dwFlags & MK_CONTROL)
		goto Cleanup;

	// Mark mouse down
	_fMouseDown = TRUE;

	if( _pobjmgr )
	{
		if( _pobjmgr->HandleDoubleClick(this, pt, dwFlags) == TRUE )
		{
			// the object subsystem handled everything
			_fMouseDown = FALSE;
			goto Cleanup;
		}
	}

	// Now update the selection
	psel = GetSel();
	AssertSz(psel, "CTxtEdit::OnLeftDblClick() - No selection object !");

	if(fEnterParaSelMode)
		psel->SelectPara(pt);
	else
		psel->SelectWord(pt);

Cleanup:
	
	return S_OK;
}

HRESULT CTxtEdit::OnTxLButtonDown(INT x, INT y, DWORD dwFlags)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnTxLButtonDown");

	BOOL		fEnterLineSelMode = FALSE;
	BOOL		fShift = dwFlags & MK_SHIFT;
	const POINT pt = {x, y};
	RECT		rc;
	COleObject *pobj;
	BOOL		fMustThaw = FALSE;

	const BOOL fTripleClick = GetTickCount() < _dwTickDblClick + DCT &&
				abs(x - _ptDblClick.x) <= cxDoubleClk &&
				abs(y - _ptDblClick.y) <= cyDoubleClk;

	// If click isn't inside view, just activate, don't select

	if(!_fFocus)					// Sets focus if not already
	{
		// we are may be removing an existing selection, so freeze
		/// the display to avoid flicker
		_pdp->Freeze();
		fMustThaw = TRUE;
		TxSetFocus();				// creates and displays caret
	}

	// Grab selection object
	CTxtSelection * const psel = GetSel();
	AssertSz(psel,"CTxtEdit::OnLeftUp() - No selection object !");

	// Check for clicks in the selection bar
	_pdp->GetViewRect(rc);

	if(!PtInRect(&rc, pt))
	{
		if(TxGetSelectionBar())			// check for click in selection bar
		{
			rc.right	= rc.left;
			rc.left		= 0;
			if(!PtInRect(&rc, pt))
				goto cancel_modes;

			// shift click in sel bar treated as normal click
			if(!(dwFlags & MK_SHIFT))
			{
				// control selbar click and triple selbar click
				// are select all
				if((dwFlags & MK_CONTROL) || fTripleClick)
				{
					psel->SelectAll();
					goto cancel_modes;
				}
				fEnterLineSelMode = TRUE;
			}
		}
		else
			goto cancel_modes;
	}
	else if(!fShift)
		psel->CancelModes();

	// let the client have a chance to handle this message if we are
	// over a link

	if( HandleLinkNotification(WM_LBUTTONDOWN, (WPARAM)dwFlags, 
			MAKELPARAM(x, y)) )
	{
		goto cancel_modes;
	}

	_fMouseDown = TRUE;						// Flag mouse down

	if( !fShift && _pobjmgr )
	{
		ClickStatus status;

		// de-activate anybody active, etc.
		status = _pobjmgr->HandleClick(this, pt);

		if( status == CLICK_OBJSELECTED )
		{
			// the object subsystem will handle resizing.
			// if not a resize we will signal start of drag
			pobj = _pobjmgr->GetSingleSelect();
			// Because HandleClick returned true, pobj better be non-null.
			Assert(pobj!=NULL);

            if (!pobj->HandleResize(pt))
			{
				_fWantDrag = !_fDisableDrag;
			}
			goto cancel_modes;
		}
		else if( status == CLICK_OBJDEACTIVATED )
		{
			goto cancel_modes;
		}
	}

	_fCapture = TRUE;						// Capture the mouse
	TxSetCapture(TRUE);

	// Check for start of drag and drop
	if(!fTripleClick && !fShift && psel->PointInSel(pt, NULL)
		&& !_fDisableDrag)
	{
		const INT vkey = GetSystemMetrics(SM_SWAPBUTTON) ? VK_RBUTTON
														 : VK_LBUTTON;
		// Assume we want a drag. If we don't CmdLeftUp() needs
		//  this to be set anyway to change the selection
		_fWantDrag = TRUE;

		goto cancel_modes;
	}


	if(fShift)								// Extend selection from current
	{										//  active end to click
		psel->InitClickForAutWordSel(pt);
		psel->ExtendSelection(pt);			
	}
	else if(fEnterLineSelMode)				// Line selection mode: select line
		psel->SelectLine(pt);
	else if(fTripleClick)					// paragraph selection mode
		psel->SelectPara(pt);
	else
		psel->SetCaret(pt);

	if( fMustThaw )
	{
		_pdp->Thaw();
	}

	return S_OK;

cancel_modes:
	psel->CancelModes();

	if (_fWantDrag)
	{
		TxSetTimer(RETID_DRAGDROP, sysparam.GetDragDelay());
		_mousePt = pt;
		_dwFkeys = dwFlags;
		_fDragged = FALSE;
	}

	if( fMustThaw )
	{
		_pdp->Thaw();
	}

	return S_OK;
}

HRESULT CTxtEdit::OnTxLButtonUp(
	INT x, 
	INT y, 
	DWORD dwFlags, 
	BOOL fReleaseCapture)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnTxLButtonUp");

	CheckRemoveContinuousScroll();

	// Remove capture before test for mouse down since we wait till
	// we get the mouse button up message to release capture since Forms
	// wants it that way.
	if(_fCapture && fReleaseCapture)
	{
		TxSetCapture(FALSE);
		_fCapture = FALSE;
	}

	if (!_fMouseDown)
	{
		// We noticed the mouse was no longer down earlier so we don't
		// need to do anything.
		return S_OK;
	}

	const BOOL fSetSel = !!_fWantDrag;
	const POINT pt = {x, y};

	// Cancel Auto Word Sel if on
	CTxtSelection * const psel = GetSel();
	AssertSz(psel,"CTxtEdit::OnLeftUp() - No selection object !");

	psel->CancelModes(TRUE);

	// Reset flags
	_fMouseDown = FALSE;
	_fWantDrag = FALSE;
	_fDragged = FALSE;
	TxKillTimer(RETID_DRAGDROP);

	// let the client handle this message if we are over a
	// link area
	if( HandleLinkNotification(WM_LBUTTONUP, (WPARAM)dwFlags, 
			MAKELPARAM(x, y)) )
	{
		return NOERROR;
	}

	// If we were in drag & drop, put caret under mouse
	if(fSetSel)
	{
		CObjectMgr* pobjmgr = GetObjectMgr();

		//If we were on an object, don't deselect it
		//by setting the caret.
		if (pobjmgr && !pobjmgr->GetSingleSelect())
		{
			psel->SetCaret(pt, TRUE);
			if(!_fFocus)
				TxSetFocus();		// create and display caret
		}
	}

	return S_OK;
}

HRESULT CTxtEdit::OnTxRButtonUp(INT x, INT y, DWORD dwFlags)
{
	const POINT pt = {x, y};
	CTxtSelection * psel;
	SELCHANGE selchg;
	HMENU hmenu = NULL;
	IOleObject * poo = NULL;
	COleObject * pobj = NULL;
	IUnknown * pUnk = NULL;
	IRichEditOleCallback * precall = NULL;

	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnTxRButtonUp");

	// make sure we have the focus
	if( !_fFocus )
	{
		TxSetFocus();
	}

	if( _fWantDrag )
	{
		_fDragged = FALSE;
		_fWantDrag = FALSE;
		TxKillTimer(RETID_DRAGDROP);
	}
		
	// Grab selection object
	psel = GetSel();
	psel->SetSelectionInfo( &selchg );

	//We need a pointer to the first object, if any, in the selection.
	if( _pobjmgr )
	{
		//If the point is in the selection we need to find out if there
		//are any objects in the selection.  If the point is not in a
		//selection but it is on an object, we need to select the object.
		if( psel->PointInSel( pt, NULL ) )
		{
			pobj = _pobjmgr->GetFirstObjectInRange( selchg.chrg.cpMin,
				selchg.chrg.cpMost );
		}
		else
		{
			//Select the object
			if( _pobjmgr->HandleClick( this, pt ) == CLICK_OBJSELECTED )
			{
				pobj = _pobjmgr->GetSingleSelect();
				// Because HandleClick returned true, pobj better be non-null.
				Assert(pobj!=NULL);
				//Refresh our information about the selection
				psel = GetSel();
				psel->SetSelectionInfo( &selchg );
			}
		}

		precall = _pobjmgr->GetRECall();
	}

	if( pobj )
	{
		pUnk = pobj->GetIUnknown();
	}

	if( pUnk )
	{
		pUnk->QueryInterface( IID_IOleObject, (void **)&poo );
	}

	if( precall )
	{
		precall->GetContextMenu( selchg.seltyp, poo, &selchg.chrg, &hmenu );
	}

	if( hmenu )
	{
		HWND hwnd, hwndParent;
		POINT ptscr;

		if( TxGetWindow( &hwnd ) == NOERROR )
		{
			ptscr.x = pt.x;
			ptscr.y = pt.y;
			ClientToScreen( hwnd, &ptscr );

			hwndParent = GetParent(hwnd);
			if( !hwndParent )
			{
				hwndParent = hwnd;
			}

			TrackPopupMenu( hmenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
				ptscr.x, ptscr.y, 0, hwndParent, NULL );
		}
		
		DestroyMenu( hmenu );
	}

	if( poo )
	{
		poo->Release();
	}
	
	return precall ? S_OK : S_FALSE;
}

HRESULT CTxtEdit::OnTxRButtonDown(INT x, INT y, DWORD dwFlags)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnTxRButtonDown");

	CTxtSelection *psel = GetSel();
	const POINT pt = {x, y};

	psel->CancelModes();

	if(psel->PointInSel(pt, NULL) && !_fDisableDrag)
	{
		_fWantDrag = TRUE;

		TxSetTimer(RETID_DRAGDROP, sysparam.GetDragDelay());
		_mousePt = pt;
		_dwFkeys = dwFlags;
		_fDragged = FALSE;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT CTxtEdit::OnTxMouseMove(INT x, INT y, DWORD dwFlags, 
		IUndoBuilder *publdr)
{
	int dx, dy;
	WORD wDragMinDist;
	DWORD vkLButton, vkRButton;
	BOOL  fLButtonDown = FALSE, fRButtonDown = FALSE;

	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnTxMouseMove");

	CTxtSelection * const psel = GetSel();

	if(!_fFocus)
		return S_OK;

	if (_fWantDrag)
	{
		wDragMinDist = sysparam.GetDragMinDist() + 3;
		dx = _mousePt.x > x ? _mousePt.x - x : x - _mousePt.x;
		dy = _mousePt.y > y ? _mousePt.y - y : y - _mousePt.y;
		if (dx < wDragMinDist && dy < wDragMinDist)
		{
			_dwFkeys = dwFlags;
			return S_OK;
		}
		_mousePt.x = x;									// Remember for later
		_mousePt.y = y;									// timer drag operation

		_fDragged = TRUE;
	}

	_mousePt.x = x;									// Remember for scrolling
	_mousePt.y = y;									//  speed, and dir calc.

	// RichEdit1.0 allows the client to process mouse moves itself if
	// we are over a link (but _not_ doing drag drop).
	if( HandleLinkNotification(WM_MOUSEMOVE, 0, MAKELPARAM(x, y)) )
	{
		return NOERROR;
	}

	// If we think the mouse is down and it really is then do special
	// processing.

	if( GetSystemMetrics(SM_SWAPBUTTON) )
	{
		vkLButton = VK_RBUTTON;
		vkRButton = VK_LBUTTON;
	}
	else
	{
		vkLButton = VK_LBUTTON;
		vkRButton = VK_RBUTTON;
	}

	fLButtonDown = (GetAsyncKeyState(vkLButton) < 0);
	if( !fLButtonDown )
	{
		fRButtonDown = (GetAsyncKeyState(vkRButton) < 0);
	}

	if( fLButtonDown || fRButtonDown )
	{
		if(_fWantDrag 
			&& !_fUsePassword
			&& !IsProtected(_fReadOnly ? WM_COPY : WM_CUT, dwFlags, 
					MAKELONG(x,y)) )
		{
			TxKillTimer(RETID_DRAGDROP);
			_ldte.StartDrag((CTxtRange *)psel, publdr);
			// the mouse button may still be down, but drag drop is over
			// so we need to _think_ of it as up.
			_fMouseDown = FALSE;

			// similarly, OLE should have nuked the capture for us, but
			// just in case something failed, release the capture.
			TxSetCapture(FALSE);
			_fCapture = FALSE;
		}
		else if (_fMouseDown)
		{
			POINT	pt = _mousePt;

			// We think the mouse is down and the mouse is down
			if (_ldte.fInDrag())
			{
				// Only do drag scrolling if a drag operation is in progress.
				_pdp->DragScroll(&pt);
			}

			AssertSz(psel,"CTxtEdit::OnMouseMove: No selection object !");
			psel->ExtendSelection(pt);				// Extend the selection

			CheckInstallContinuousScroll ();
		}
	}
	else if ( GetAsyncKeyState( VK_MBUTTON ) < 0 )
	{
		; /* do nothing, middle button pressed. */
	}
	else
	{
		if (_fMButtonCapture)
		{
			OnTxMButtonUp (x, y, dwFlags);
		}
		if (_fMouseDown )
		{
			// Although we thought the mouse was down, at this moment it
			// clearly is not. Therefore, we pretend we got a mouse up
			// message and clear our state to get ourselves back in sync 
			// with what is really happening.
			OnTxLButtonUp(x, y, dwFlags, FALSE);
		}
		
	}

	// Either a drag was started or the mouse button was not down. In either
	// case, we want no longer to start a drag so we set the flag to false.
	_fWantDrag = FALSE;

	return S_OK;
}

/*
 *	CTxtEdit::OnTxMButtonDown (INT x, INT y, DWORD dwFlags)
 *
 *	@mfunc
 *		The user pressed the middle mouse button, setup to do
 *		continuous scrolls, which may in turn initiate a timer
 *		for smooth scrolling.
 */
HRESULT CTxtEdit::OnTxMButtonDown (INT x, INT y, DWORD dwFlags)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnTxMButtonDown");

#ifndef MACPORT
	POINT	mDownPt = {x,y};

	if ( mouse.MagellanStartMButtonScroll( *this, mDownPt ) )
	{
		TxSetCapture(TRUE);

		_fCapture			= TRUE;							// Capture the mouse
		_fMouseDown			= TRUE;
		_fMButtonCapture	= TRUE;
	}
#endif

	return S_OK;
}

/*
 *	CTxtEdit::OnTxMButtonUp (INT x, INT y, DWORD dwFlags)
 *
 *	@mfunc
 *		Remove timers and capture assoicated with a MButtonDown
 *		message.
 */
HRESULT CTxtEdit::OnTxMButtonUp (INT x, INT y, DWORD dwFlags)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnTxMButtonUp");

#ifndef MACPORT
	mouse.MagellanEndMButtonScroll( *this );
#endif

	if(_fCapture)
	{
		TxSetCapture(FALSE);
	}
	_fCapture			= FALSE;
	_fMouseDown			= FALSE;
	_fMButtonCapture	= FALSE;


	return S_OK;
}

/*
 *	CTxtEdit::CheckInstallContinuousScroll ( )
 *
 *	@mfunc
 *		There are no events that inform the app on a regular
 *		basis that a mouse button is down. This timer notifies
 *		the app that the button is still down, so that scrolling can
 *		continue.
 */
void CTxtEdit::CheckInstallContinuousScroll ( )
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::CheckInstallContinuousScroll");

	if ( !_fContinuousScroll && TxSetTimer(RETID_AUTOSCROLL, cmsecScrollInterval) )
	{
		_fContinuousScroll = TRUE;
	}
}

/*
 *	CTxtEdit::CheckRemoveContinuousScroll ( )
 *
 *	@mfunc
 *		The middle mouse button, or drag button, is up
 *		remove the continuous scroll timer.
 */
void CTxtEdit::CheckRemoveContinuousScroll ( )
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::CheckRemoveContinuousScroll");

	if(	_fContinuousScroll )
	{
		TxKillTimer(RETID_AUTOSCROLL);
		_fContinuousScroll = FALSE;
	}
}

/*
 *	CTxtEdit::OnTxTimer(UINT idTimer)
 *
 *	@mfunc
 *		Handle timers for doing background recalc and scrolling.
 */
HRESULT CTxtEdit::OnTxTimer(UINT idTimer)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnTxTimer");

	HRESULT	result = S_OK;


	switch (idTimer)
	{
		case RETID_BGND_RECALC:
		{
			_pdp->StepBackgroundRecalc();
			break;
		}

#ifndef _MAC
		case RETID_MAGELLANTRACK:
		{
			mouse.TrackUpdateMagellanMButtonDown( *this, _mousePt );
			break;
		}

#endif // _MAC

		case RETID_AUTOSCROLL:						// Continuous scrolling.
		{
													// Do a select drag scroll.
			OnTxMouseMove( _mousePt.x, _mousePt.y, 0, NULL );
			break;
		}

#ifndef _MAC
		case RETID_SMOOTHSCROLL:					// Smooth scrolling
		{
			if ( _fMButtonCapture )					// HACK, only 1 timer!
			{										// delivered on Win95
													// when things get busy.
				mouse.TrackUpdateMagellanMButtonDown( *this, _mousePt );	
			}

			if ( _pdp->IsSmoothVScolling() )		// Test only because of
			{										//  above HACK!!
				_pdp->SmoothVScrollUpdate();
			}
		}
		break;
#endif // _MAC
		case RETID_DRAGDROP:
		{
			TxKillTimer(RETID_DRAGDROP);
			if(_fWantDrag 
				&& _fDragged
				&& !_fUsePassword
				&& !IsProtected(_fReadOnly ? WM_COPY : WM_CUT,
				                          _dwFkeys,
										  MAKELONG(_mousePt.x,_mousePt.y)) )
			{
				CGenUndoBuilder undobldr(this, UB_AUTOCOMMIT);
				_ldte.StartDrag((CTxtRange *)GetSel(), &undobldr);
				_fWantDrag = FALSE;
				_fDragged = FALSE;
				TxSetCapture(FALSE);
				_fCapture = FALSE;

			}
			break;
		}
		default:
		{
			result = S_FALSE;
			break;
		}
	}

	return result;
}


/////////////////////////// Keyboard Commands ////////////////////////////////

/*
 *	CTxtEdit::OnTxKeyDown(vkey, dwFlags, publdr)
 *
 *	@mfunc
 *		Handle WM_KEYDOWN message
 *
 *	@rdesc
 *		HRESULT with the following values:
 *
 *		S_OK				if key was understood and consumed
 *		S_MSG_KEY_IGNORED	if key was understood, but not consumed
 *		S_FALSE				if key was not understood (and not consumed)
 */
HRESULT CTxtEdit::OnTxKeyDown(
	WORD		  vkey,		//@parm Virtual key code
	DWORD		  dwFlags,	//@parm lparam of WM_KEYDOWN msg
	IUndoBuilder *publdr)	//@parm Undobuilder to receive antievents
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnTxKeyDown");

	BOOL fRet	= FALSE;			// Converted to HRESULT on return
	BOOL fCtrl  = FALSE;
	BOOL fShift;

	// If dragging or Alt key down, just look for ESCAPE
	// FUTURE (alexgo): if Alt key is down, we should never come here (would generate
	// WM_SYSKEYDOWN message), so GetKeyState() can probably be omitted...
	if(_fMouseDown || (GetKeyState(VK_MENU) & 0x8000))
	{
		if (vkey == VK_ESCAPE)
		{
			POINT pt;
			// Cancel drag select or drag & drop
			GetCursorPos(&pt);
			OnTxLButtonUp(pt.x, pt.y, 0, TRUE);
		}
		return S_OK;
	}
	
	CTxtSelection * const psel = GetSel();
	AssertSz(psel,"CTxtEdit::OnKeyDown() - No selection object !");

	if(GetKeyState(VK_CONTROL) & 0x8000)
	{
		fCtrl = TRUE;

 		if (IsIMEComposition())
		{
			// During IME Composition, there are some key events we should
			// not handle.  Also, there are other key events we need to handle by
			// terminating the IME composition first.
			BOOL bDontHandle = FALSE;

			switch(vkey)
			{
			case VK_TAB:
		   	case VK_CLEAR:
			case VK_NUMPAD5:
			case 'A':						// Ctrl-A => pselect all
			case 'C':						// Ctrl-C => copy
			case 'X':						// Ctrl-X => cut
			case 'Y':						// Ctrl-Y => redo
				bDontHandle = TRUE;
				break;

			case 'V':						// Ctrl-V => paste
			case 'Z':						// Ctrl-Z => undo	
				_ime->TerminateIMEComposition( *this );

				// early exist for undo case
				if (vkey == 'Z')
					bDontHandle = TRUE;
				break;

			default:
				// we can goto cont since we have handled all the cases
				// for the below switch block.
				goto cont; 
			}

			if (bDontHandle)
				return S_OK;
		}

		switch(vkey)
		{
		case VK_TAB:
			return OnTxChar(VK_TAB, dwFlags, publdr);

		case VK_CLEAR:
		case VK_NUMPAD5:
		case 'A':						// Ctrl-A => pselect all
			psel->SelectAll();
			break;
	
		case 'C':						// Ctrl-C => copy
CtrlC:		if( !_fUsePassword 
				&& !IsProtected( WM_COPY, 0,  0) )
			{
				_ldte.CopyRangeToClipboard((CTxtRange *)psel);
			}
			break;
	
		case 'V':						// Ctrl-V => paste
CtrlV:		if( IsntProtectedOrReadOnly( WM_PASTE, 0, 0) )
			{
				PasteDataObjectToRange(NULL, (CTxtRange *)psel, 0, NULL, 
					publdr, PDOR_NONE);

				// to match the Word UI, better go ahead and update the window
				TxUpdateWindow();
			}
			break;
	
		case 'X':						// Ctrl-X => cut
CtrlX:		if( !_fUsePassword 
				&& IsntProtectedOrReadOnly( WM_CUT, 0, 0) )
			{
				_ldte.CutRangeToClipboard((CTxtRange *)psel, publdr);
			}
			break;
	
		case 'Z':						// Ctrl-Z => undo
			if (_pundo && !_fReadOnly )
			{
				PopAndExecuteAntiEvent(_pundo, 0);
			}
			break;

		case 'Y':						// Ctrl-Y => redo
			if( _predo && !_fReadOnly )
			{
				PopAndExecuteAntiEvent(_predo, 0);
			}
			break; 

		default:
			goto cont;
		}
		return S_OK;
	}

cont:
	fShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;

	if (IsIMEComposition())
	{
		// During IME Composition, there are some key events we should
		// not handle.  Also, there are other key events we need to handle by
		// terminating the IME composition first.
		BOOL bDontHandle = FALSE;				

		switch(vkey)
		{
		case VK_BACK:
		case VK_F16:
			bDontHandle = TRUE; 
			break;

		case VK_INSERT:								// Ins
			if(fShift)								// Shift-Ins --> paste
				_ime->TerminateIMEComposition( *this );
			else 
				bDontHandle = TRUE;
			break;

		case VK_LEFT:								// Left arrow
		case VK_RIGHT:								// Right arrow
		case VK_UP:									// Up arrow
		case VK_DOWN:								// Down arrow
		case VK_HOME:								// Home
		case VK_END:								// End
		case VK_PRIOR:								// PgUp
		case VK_NEXT:								// PgDn
		case VK_DELETE:								// Del
		// Ctrl-Return generates Ctrl-J (LF), treat it as an ordinary return
		case CTRL('J'):
		case VK_RETURN:
			_ime->TerminateIMEComposition( *this );
			break;
		}

		if (bDontHandle)
			return S_MSG_KEY_IGNORED; 
	}

	psel->SetExtend(fShift);

	switch(vkey)
	{
	case VK_BACK:
	case VK_F16:
		if(_fReadOnly)
		{	
			Sound();
			fRet = TRUE;
		}
		else if(IsntProtectedOrReadOnly(WM_KEYDOWN, VK_BACK, dwFlags))
		{
			fRet = psel->Backspace(fCtrl, publdr);
		} 
		break;

	case VK_INSERT:								// Ins
		if(fShift)								// Shift-Ins
			goto CtrlV;							// Alias for Ctrl-V
		if(fCtrl)								// Ctrl-Ins
			goto CtrlC;							// Alias for Ctrl-C

		if(!_fReadOnly)							// Ins
			_fOverstrike = !_fOverstrike;		// Toggle Ins/Ovr
		fRet = TRUE;
		break;

	case VK_LEFT:								// Left arrow
		fRet = psel->Left(fCtrl);
		break;

	case VK_RIGHT:								// Right arrow
		fRet = psel->Right(fCtrl);
		break;

	case VK_UP:									// Up arrow
		fRet = psel->Up(fCtrl);
		break;

	case VK_DOWN:								// Down arrow
		fRet = psel->Down(fCtrl);
		break;

	case VK_HOME:								// Home
		fRet = psel->Home(fCtrl);
		break;

	case VK_END:								// End
		fRet = psel->End(fCtrl);
		break;

	case VK_PRIOR:								// PgUp
		fRet = psel->PageUp(fCtrl);
		break;

	case VK_NEXT:								// PgDn
		fRet = psel->PageDown(fCtrl);
		break;

	case VK_DELETE:								// Del
		if(fShift)								// Shift-Del
			goto CtrlX;							// Alias for Ctrl-X

		if(IsntProtectedOrReadOnly(WM_KEYDOWN, VK_DELETE, dwFlags))
			psel->Delete(fCtrl, publdr);
		fRet = TRUE;
		break;

	case CTRL('J'):								// Ctrl-Return gives Ctrl-J
	case VK_RETURN:								//  (LF), treat it as return
		if(!TxGetMultiLine())
		{
			Sound();
			return S_FALSE;
		}
		SetCursor(0);

		if(IsntProtectedOrReadOnly(WM_CHAR, VK_RETURN, dwFlags))
			psel->InsertEOP(publdr);
	
		fRet = TRUE;
		break;
		
	case VK_PROCESSKEY:
		// We will receive this case when handling
		// IME input only. Check to see if protected is on
		// If so, we will ingore IME input
		if(!IsntProtectedOrReadOnly(WM_CHAR, VK_RETURN, dwFlags))
		{
			HWND hwnd;
			
			TxGetWindow( &hwnd );
			return ( IgnoreIMEInput( hwnd, *this, dwFlags ) );	
		}
		return S_FALSE;

	case VK_KANA:
		// for Japanese, this virtual key indicates Kana mode.  
		// Check the toggle bit to see if it is on or offf.
		// If on, Kana characters (single byte Japanese char) are coming in via WM_CHAR.
		_fKANAMode = (GetKeyState(VK_KANA) & 0x1) != 0;
		return S_OK;

	case VK_KANJI:
		// for Korean, need to convert the next Korean Hangeul character to Hanja
		if(_KOREAN_CP == GetKeyboardCodePage() && 
			IsntProtectedOrReadOnly(WM_KEYDOWN, VK_KANJI, dwFlags))
			fRet = IMEHangeulToHanja ( *this, *publdr );
		break;

	default:
		return S_FALSE;
	}
	
	psel->SetExtend(FALSE);

	return fRet ? S_OK : S_MSG_KEY_IGNORED;
}

/*
 *	CTxtEdit::OnTxChar (vkey, dwFlags, publdr)
 *
 *	@mfunc
 *		Handle WM_CHAR message
 *
 *	@rdesc
 *		HRESULT with the following values:
 *
 *		S_OK				if key was understood and consumed
 *		S_MSG_KEY_IGNORED	if key was understood, but not consumed
 *		S_FALSE				if key was not understood (and not consumed)
 */
HRESULT CTxtEdit::OnTxChar(
	WORD		  vkey,		//@parm Translated key code
	DWORD		  dwFlags,	//@parm lparam of WM_KEYDOWN msg
	IUndoBuilder *publdr)	//@parm Undobuilder to receive antievents
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnTxChar");

	if (_fMouseDown || vkey == VK_ESCAPE ||	// Ctrl-Backspace generates VK_F16
		vkey == VK_BACK || vkey==VK_F16)	// Eat it since we process it
	{										//  in WM_KEYDOWN
		return S_OK;
	}

	CTxtSelection * const psel = GetSel();
	AssertSz(psel,
		"CTxtEdit::OnChar() - No selection object !");
	psel->SetExtend(FALSE);					// Shift doesn't mean extend for
											//  WM_CHAR
	if(_fReadOnly && vkey != 3)				// Don't allow input if read only,
	{										//  but allow copy (Ctrl-C)
		if(vkey >= ' ')
			Sound();
		return S_MSG_KEY_IGNORED;
	}

	if(vkey >= ' ' || vkey == VK_TAB)
	{
		SetCursor(0);

		if(IsntProtectedOrReadOnly(WM_CHAR, vkey, dwFlags))
		{
			psel->PutChar((TCHAR)vkey, _fOverstrike, publdr);
		}
	}
	return S_OK;
}


LRESULT CTxtEdit::OnTxSysKeyDown(WORD vkey, DWORD dwFlags)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnTxSysKeyDown");

	LRESULT lres = S_FALSE;
	
	if(vkey == VK_BACK && (dwFlags & SYS_ALTERNATE))
	{
		if( _pundo && _pundo->CanUndo())
		{
			if( PopAndExecuteAntiEvent(_pundo, 0) == NOERROR )
			{	
				lres = NOERROR;
			}
			else
			{
				lres = S_MSG_KEY_IGNORED;
			}
		}
		else
			Sound();
	}
	else if( (vkey == VK_F10 &&// F10
			!(dwFlags & SYS_PREVKEYSTATE) &&		// Key previously up
			(GetKeyState(VK_SHIFT) & 0x8000)))		// Shift is down
	{
		HandleKbdContextMenu();
	}

	return lres ? S_OK : S_MSG_KEY_IGNORED;
}

/////////////////////////////// Other system events //////////////////////////////

HRESULT CTxtEdit::OnContextMenu(LPARAM lparam)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnContextMenu");

	POINT pt;

	pt.x = LOWORD(lparam);
	pt.y = HIWORD(lparam);

	if( TxScreenToClient(&pt) )
	{
		return OnTxRButtonUp(pt.x, pt.y, 0);
	}
	return S_FALSE;
}

/*
 *	CTxtEdit::HandleKbdContextMenu
 *
 *	@mfunc	decides where to put the context menu on the basis of where the
 *			the selection is.  Useful for shift-F10 and VK_APPS, where
 *			we aren't given a location.
 *
 *	@rdesc	void
 */
void CTxtEdit::HandleKbdContextMenu(void)
{
	POINT	pt;
	RECT	rc;
	const CTxtSelection * const psel = GetSel();

	// Figure out where selection ends and put context menu near it
	if(_pdp->PointFromTp(*psel, NULL, FALSE, pt, NULL, TA_TOP) < 0)
		return;

	// Make sure point is still within bounds of edit control
	_pdp->GetViewRect(rc);
	
	if (pt.x < rc.left)
		pt.x = rc.left;
	if (pt.x > rc.right - 2)
		pt.x = rc.right - 2;
	if (pt.y < rc.top)
		pt.y = rc.top;
	if (pt.y > rc.bottom - 2)
		pt.y = rc.bottom - 2;

	OnTxRButtonUp(pt.x, pt.y, 0);
}


/////////////////////////////// Format Range Commands //////////////////////////////


LONG CTxtEdit::OnFormatRange(
	FORMATRANGE *pfr, 
	SPrintControl prtcon,
	HDC hdcMeasure,
	LONG xMeasurePerInch,
	LONG yMeasurePerInch)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnFormatRange");

	// Even if there is 0 text, we want to print the control so that it will
	// fill the control with background color.
	// Use Adjusted Text Length.  Embedded objects using RichEdit will get the empty
	// document they expect and will create a default size document.
	if(!pfr || 
		((pfr->chrg.cpMin >= (LONG)GetAdjustedTextLength()) && !prtcon._fPrintFromDraw))
	{	// we're done formatting, get rid of our printer's display context.
		delete _pdpPrinter;
		_pdpPrinter = NULL;

		return GetAdjustedTextLength();
	}

	LONG lRetVal = -1;
	BOOL fSetDCWorked = FALSE;
	HDC hdcLocal = pfr->hdc;

	// first time in with this printer, set up a new display context.
	// IMPORTANT: proper completion of the printing process is required
	// to dispose of this context and begin a new context.
	// This is implicitly done by printing the last character, or
	// sending an EM_FORMATRANGE message with pfr equal to NULL.
	if ( NULL == _pdpPrinter )
	{
		_pdpPrinter = new CDisplayPrinter (this, hdcLocal,
				pfr->rc.right - pfr->rc.left /* x width max */,
				pfr->rc.bottom - pfr->rc.top, /* y height max */ 
				prtcon);

		_pdpPrinter->Init();

		// Future: (ricksa) This is a really yucky way to pass the draw info
		// to the printer but it was quick. We want to make this better.
		_pdpPrinter->ResetDrawInfo(_pdp);

		// Set the temporary zoom factor (if there is one).
		_pdpPrinter->SetTempZoomDenominator(
			_pdp->GetTempZoomDenominator());
	}
	else
	{
		_pdpPrinter->SetPrintDimensions(&pfr->rc);
	}

	// We set the DC everytime because it could have changed.
	if (GetDeviceCaps(hdcLocal, TECHNOLOGY) != DT_METAFILE)
	{
		// This is not a metafile so do the normal thing
		fSetDCWorked = _pdpPrinter->SetDC(hdcLocal);
	}
	else
	{
		// Is the measure DC already set up?
		if (NULL == hdcMeasure)
		{
			// We need to set up a metafile with an HDC we can use for
			// measuring. It is important to note the hack going on here.
			// Richedit 1.0 assumed that it could map metafiles using TWIPS.
			// Therefore, we pass the TRUE flag into CreateMeasureDC which
			// causes a measure DC to be created which is based on TWIPS.
			hdcMeasure = CreateMeasureDC(hdcLocal, NULL, TRUE, pfr->rcPage.left, 
				pfr->rcPage.top, pfr->rcPage.right - pfr->rcPage.left,
					pfr->rcPage.bottom - pfr->rcPage.top, &xMeasurePerInch, 
						&yMeasurePerInch);
		}

		// Were we able to set up a measure DC?
		if (hdcMeasure != NULL)
		{
			_pdpPrinter->SetMetafileDC(hdcLocal, hdcMeasure,
				xMeasurePerInch, yMeasurePerInch);
			fSetDCWorked = TRUE;
		}
	}

	if (fSetDCWorked)
	{
		// we set this everytime because it could have changed.
		if ( _pdpPrinter->SetTargetDC( pfr->hdcTarget ) )
		{
			LONG	cpReturn;


			//	Format another, single page worth of text.
			cpReturn = _pdpPrinter->FormatRange( pfr->chrg.cpMin, pfr->chrg.cpMost);

			if (!prtcon._fPrintFromDraw)
			{
				// after formatting, we know where the bottom is. But we only 
				// want to set this if we are writing a page rather than
				// displaying a control on the printer.
				pfr->rc.bottom = INT 
					(pfr->rc.top + _pdpPrinter->DYtoLY( _pdpPrinter->GetHeight()) );
			}

			// remember this in case the host wishes to do its own banding.
			_pdpPrinter->SetPrintView( pfr->rc );	// we need to save this for OnDisplayBand.

			_pdpPrinter->SetPrintPage( pfr->rcPage );

			// if we're asked to render, then render the entire page in one go.
			if(prtcon._fDoPrint && ((cpReturn > 0) || prtcon._fPrintFromDraw))
			{
				OnDisplayBand( &pfr->rc, prtcon._fPrintFromDraw );

				// note: we can no longer call OnDisplayBand without reformating.
				_pdpPrinter->Clear(AF_DELETEMEM);	
			}

			lRetVal = cpReturn;
		}

	}

	if (hdcMeasure)
	{
		TxReleaseMeasureDC(hdcMeasure);
	}

	return lRetVal;
}

// Keep the compiler quiet about xPhys and yPhys below.
// The compiler complains incorrectly about possible use of uninitialized variable.
#pragma warning (disable : 4701)
BOOL CTxtEdit::OnDisplayBand(const RECT *prc, BOOL fPrintFromDraw)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnDisplayBand");

	RECT	rc, rcPrint;

	HDC		hdcPrinter;

	// Make sure OnFormatRange was called and that it actually rendered something.
	if(!_pdpPrinter || !_pdpPrinter->Count())
		return FALSE;

	// proportionally map to printers extents.
	rc.left		= (INT) _pdpPrinter->LXtoDX(prc->left);
	rc.right	= (INT) _pdpPrinter->LXtoDX(prc->right);
	rc.top		= (INT) _pdpPrinter->LYtoDY(prc->top);
	rc.bottom	= (INT) _pdpPrinter->LYtoDY(prc->bottom);

	rcPrint			= _pdpPrinter->GetPrintView();
	rcPrint.left	= (INT) _pdpPrinter->LXtoDX(rcPrint.left);
	rcPrint.right	= (INT) _pdpPrinter->LXtoDX(rcPrint.right);
	rcPrint.top		= (INT) _pdpPrinter->LYtoDY(rcPrint.top);
	rcPrint.bottom	= (INT) _pdpPrinter->LYtoDY(rcPrint.bottom);

	// Get the DC for the printer because we use it below.
	hdcPrinter = _pdpPrinter->GetDC();

	if (fPrintFromDraw)
	{
		// We need to take the view inset into account
		_pdpPrinter->GetViewRect(rcPrint, &rcPrint);
	}

	// Render this band.
	_pdpPrinter->Render(rc, rcPrint);

	return TRUE;
}
#pragma warning (default : 4701)



//////////////////////////////// Protected ranges //////////////////////////////////

/*
 *	CTxtEdit::IsProtected (msg, wparam, lparam)
 *
 *	@mfunc
 *		Find out if selection is protected
 *
 *	@rdesc
 *		TRUE iff 1) control is read-only or 2) selection is protected and
 *		parent query says to protect
 */
BOOL CTxtEdit::IsProtected(
	UINT	msg, 		//@parm	Message id
	WPARAM	wparam, 	//@parm WPARAM from window's message
	LPARAM	lparam)		//@parm LPARAM from window's message
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::IsProtected");
	
	LONG iDirection = 0;

	if( _dwEventMask & ENM_PROTECTED )
	{
		CTxtSelection *psel = GetSel();

		// there are a few special cases to consider, namely
		// backspacing into a protected range, 'del'eting into
		// a protected range, and type with overstrike into a protected
		// range.
		if( (msg == WM_KEYDOWN && (wparam == VK_BACK || wparam == VK_F16)) )
		{
			// check for the format behind the selection, if we are trying to 
			// backspace an insertion point.
			iDirection = -1;
		}
		else if( (msg == WM_KEYDOWN && wparam == VK_DELETE) || 
			(_fOverstrike && msg == WM_CHAR) )
		{
			iDirection = 1;
		}

		int iProt;

		if(psel && ((iProt = psel->IsProtected(iDirection)) == CTxtRange::PROTECTED_YES ||
			(iProt == CTxtRange::PROTECTED_ASK && 
				QueryUseProtection(psel, msg, wparam, lparam))))
		// N.B.  the preceding if statement assumes that IsProtected returns a tri-value
		{
			return TRUE;
		}
		// fall through to return FALSE
	}
	return FALSE;
}

/*
 *	CTxtEdit::IsntProtectedOrReadOnly (msg, wparam, lparam)
 *
 *	@mfunc
 *		Find out if selection isn't protected or read only. If it is, 
 *		ring bell
 *
 *	@rdesc
 *		TRUE iff 1) control isn't read-only and 2) selection either isn't
 *		protected or parent query says not to protect
 *
 *	@devnote	This function is useful for UI operations (like typing).
 *
 */
BOOL CTxtEdit::IsntProtectedOrReadOnly(UINT msg, WPARAM wparam, LPARAM lparam)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::IsProtected");

	if( !_fReadOnly && !IsProtected(msg, wparam, lparam))
		return TRUE;

	if (_fReadOnly)
		Sound();

	return FALSE;
}

/*
 *	CTxtEdit::IsProtectedRange (msg, wparam, lparam, prg)
 *
 *	@mfunc
 *		Find out if range prg is protected
 *
 *	@rdesc
 *		TRUE iff control is read-only or range is protected and parent
 *		query says to protect
 */
BOOL CTxtEdit::IsProtectedRange(
	UINT		msg, 		//@parm	Message id
	WPARAM		wparam, 	//@parm WPARAM from window's message
	LPARAM		lparam,		//@parm LPARAM from window's message
	CTxtRange *	prg)		//@parm Range to examine
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::IsProtectedRange");
	
	int iProt;

	if((_dwEventMask & ENM_PROTECTED) && 
		((iProt = prg->IsProtected(0)) == CTxtRange::PROTECTED_YES ||
			(iProt == CTxtRange::PROTECTED_ASK && 
				QueryUseProtection(prg, msg, wparam, lparam))))
	// N.B.  the preceding if statement assumes that IsProtected returns a tri-value
	{
		return TRUE;
	}

	return FALSE;
}


/////////////////////////////// Private IUnknown //////////////////////////////

HRESULT __stdcall CTxtEdit::CUnknown::QueryInterface(
	REFIID riid, 
	void **ppvObj)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::CUnknown::QueryInterface");

	HRESULT hr = E_NOINTERFACE;
	*ppvObj = NULL;
	CTxtEdit *ped = (CTxtEdit *)GETPPARENT(this, CTxtEdit, _unk);

	if (IsEqualIID(riid, IID_IUnknown) 
		|| IsEqualIID(riid, IID_ITextServices)) 
	{
		*ppvObj = (ITextServices *)ped;
	}
	else if( IsEqualIID(riid, IID_IDispatch) )
	{
		*ppvObj = (IDispatch *)ped;
	}
	else if( IsEqualIID(riid, IID_ITextDocument) )
	{
		*ppvObj = (ITextDocument *)ped;
	}
	else if( IsEqualIID(riid, IID_IRichEditOle) )
	{
		*ppvObj = (IRichEditOle *)ped;
	}
	else if( IsEqualIID(riid, IID_IRichEditOleCallback) )
	{
		//
		// NB!! Returning this pointer in our QI is 
		// phenomenally bogus; it breaks fundamental COM
		// identity rules (granted, not many understand them!).
		// Anyway, RichEdit 1.0 did this, so we better.
		//

		TRACEWARNSZ("Returning IRichEditOleCallback interface, COM "
			"identity rules broken!");

		*ppvObj = ped->GetRECallback();
	}

	if( *ppvObj )
	{
		((IUnknown *) *ppvObj)->AddRef();
		hr = S_OK;
	}

	return hr;
}

ULONG __stdcall	CTxtEdit::CUnknown::AddRef()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::CUnknown::AddRef");

	return ++_cRefs;
}

ULONG __stdcall CTxtEdit::CUnknown::Release()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::CUnknown::Release");

	// the call manager will take care of deleting our instance if appropriate.
	CTxtEdit *ped = GETPPARENT(this, CTxtEdit, _unk);
	CCallMgr callmgr(ped);

	ULONG culRefs =	--_cRefs;

	if( culRefs == 0 )
	{
		// even though we don't delete ourselves now, dump the callback
		// if we have it.  This make implementation a bit easier on clients.

		if( ped->_pobjmgr )
		{
			ped->_pobjmgr->SetRECallback(NULL);
		}

		// make sure our timers are gone
		
		ped->TxKillTimer(RETID_AUTOSCROLL);
		ped->TxKillTimer(RETID_DRAGDROP);
		ped->TxKillTimer(RETID_BGND_RECALC);
		ped->TxKillTimer(RETID_SMOOTHSCROLL);
		ped->TxKillTimer(RETID_MAGELLANTRACK);
	}

	return culRefs;
}


WORD		wConvScroll(WORD wparam);

#ifdef DBCS
LOCAL USHORT	usDBCSCombine (HWND, CHAR);
#endif


/*
 *  ValidateTextRange(pstrg)
 *
 *  Purpose:
 *	  Makes sure that an input text range structure makes sense.
 *
 *  Arguments:
 *	  pstrg   pointer to a text range structure.
 *
 *  Returns:
 *	  Size of the buffer required to accept copy of data or -1 if all the
 *	  data in the control is requested. 
 *
 *  Notes:
 *	  This is used both in this file and in the RichEditANSIWndProc
 */
LONG ValidateTextRange(TEXTRANGE *pstrg)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "ValidateTextRange");

					
	// Validate that the input structure makes sense. In the first
	// place it must be big enough. Secondly, the values must sense.
	// Remember that if the cpMost field is -1 and the cpMin field
	// is 0 this means that the call wants the entire buffer. 
	if (IsBadReadPtr(pstrg, sizeof(TEXTRANGE))
		|| (((pstrg->chrg.cpMost < 1) || (pstrg->chrg.cpMin < 0)
				|| (pstrg->chrg.cpMost <= pstrg->chrg.cpMin))
			&& !((pstrg->chrg.cpMost == -1) && (pstrg->chrg.cpMin == 0))))
	{
		// This isn't valid so tell the caller we didn't copy
		// any data.
		return 0;
	}

	// Calculate the size of the buffer that we need on return.
	return pstrg->chrg.cpMost - pstrg->chrg.cpMin;
   
}



////////////////////////////////////  Selection  /////////////////////////////////////


CTxtSelection * CTxtEdit::GetSel()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::GetSel");

	if (NULL == _psel)
	{
		// There is no selection object available so create it.
		_psel = new CTxtSelection(_pdp);

		if (_psel)
		{
			// Set the reference count to 1.
			_psel->AddRef();
		}
	}

	// It is the caller's responsiblity to notice that an error occurred
	// in the allocation of the selection object.
	return _psel;
}

void CTxtEdit::DiscardSelection()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::DiscardSelection");

	if (_psel)
	{
		_psel->Release();

		if (_psel != NULL)
		{
			// The text services reference is not the last reference to the 
			// selection. We could keep track of the fact that text services
			// has released its reference and when text services gets a 
			// reference again, do the AddRef there so that if the last 
			// reference went away while we were still inactive, the selection
			// object would go away. However, it is seriously doubtful that 
			// such a case will be very common. Therefore, just do the simplest
			// thing and put our reference back.
			_psel->AddRef();
		}
	}
}

void CTxtEdit::GetSelRangeForRender(LONG *pcpSelMin, LONG *pcpSelMost)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::GetSelRangeForRender");

	// If we have no selection or we are not active and the selection
	// has been requested to be hidden, there is no selection so we
	// just return 0's.
	if ((NULL == _psel) 
		|| (!_fInPlaceActive && _fHideSelection))
	{
		*pcpSelMin = 0;
		*pcpSelMost = 0;
		return;
	}

	// Otherwise return the state of the current selection.
	*pcpSelMin = _psel->GetScrSelMin();
	*pcpSelMost = _psel->GetScrSelMost();
}

LONG CTxtEdit::OnGetSelText(TCHAR *psz)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnGetSelText");

	CHARRANGE crSel;							// crSel = current selection
	OnExGetSel(&crSel);							// length + 1 for the null
	return GetTextRange(crSel.cpMin, crSel.cpMost - crSel.cpMin + 1, psz);
}

void CTxtEdit::OnExGetSel(CHARRANGE *pcrSel) const
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnExGetSel");

	pcrSel->cpMin  = GetSelMin();
	pcrSel->cpMost = GetSelMost();
}

LRESULT CTxtEdit::OnGetSel(LONG *pcpMin, LONG *pcpMost) const
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnGetSel");

	CHARRANGE crSel;

	OnExGetSel(&crSel);
	if(pcpMin)
		*pcpMin = crSel.cpMin;
	if(pcpMost)
		*pcpMost = crSel.cpMost;

	return (crSel.cpMost > 65535l)	? (LRESULT) -1
				: MAKELRESULT((WORD) crSel.cpMin, (WORD) crSel.cpMost);
}

/*
 *	CTxtEdit::OnSetSel (cpMin, cpMost)
 *
 *	Purpose:
 *		implements the EM_SETSEL message
 *
 *	Algorithm:
 *		there are three basic cases to handle
 *
 *		cpMin >= 0, cpMost >= 0		-- treat as cpMin, cpMost
 *
 *		cpMin == 0, cpMost == -1	-- this is the SelectAll case
 *
 *		cpMin == -1, cpMost ???		-- this means collapse the selection
 *									   to an insertion point.  If cpMin
 *									   is greater than cpMost, then the
 *									   selection's cpMin is collapsed to.
 *									   Otherwise, collapse to the greater
 *									   end.	
 *										    
 *		cpMin > 0, cpMost == -1		-- select from cpMin to the end.			
 */
void CTxtEdit::OnSetSel(LONG cpMin, LONG cpMost)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnSetSel");

	// Since this is only called from the window proc, we are always active
	Assert( GetSel() );
	
	CTxtSelection * const psel = GetSel();

	if( cpMost == -1 )
	{
		cpMost = GetTextLength();
		if( cpMin < 0 )
		{
			cpMin = 0;
		}
		// fall through to the SetSelection call below
	}
	else if( cpMin == -1 )
	{
		LONG	a, b;

		psel->GetRange(a, b);

		// this is a bit weird, but basically implements the
		// algorithm noted above; i.e. collapse to the end
		// indicated by the "greater" limit

		cpMin = cpMost = (cpMin > cpMost) ? a : b;

		// fall through to the SetSelection call below
	}
	
	psel->SetSelection(cpMin, cpMost);
}

///////////////////////////////  DROP FILES support  //////////////////////////////////////
#ifndef MACPORT

LRESULT CTxtEdit::InsertFromFile ( LPCTSTR lpFile )
{
	REOBJECT		reobj;
	LPRICHEDITOLECALLBACK const precall = GetRECallback();
	HRESULT			hr = NOERROR;

	if(!precall)
		return E_NOINTERFACE;

	ZeroMemory(&reobj, sizeof(REOBJECT));
	reobj.cbStruct = sizeof(REOBJECT);

	// Get storage for the object from the application
	hr = precall->GetNewStorage(&reobj.pstg);
	if (hr)
	{
		TRACEERRORSZ("GetNewStorage() failed.");
		goto err;
	}

	// Create an object site for the new object
	GetClientSite(&reobj.polesite);
	if (hr)
	{
		TRACEERRORSZ("GetClientSite() failed.");
		goto err;
	}
	
	hr = OleCreateLinkToFile(lpFile, IID_IOleObject, OLERENDER_DRAW,
				NULL, NULL, reobj.pstg, (LPVOID*)&reobj.poleobj);	
	if (hr)
	{
		TRACEERRORSZ("Failure creating link object.");
		goto err;
	}

 	reobj.cp = REO_CP_SELECTION;
	reobj.dvaspect = DVASPECT_CONTENT;

 	//Get the clsid of the object.
	hr = reobj.poleobj->GetUserClassID(&reobj.clsid);
	if (hr)
	{
		TRACEERRORSZ("GetUserClassID() failed.");
		goto err;
	}

	// Let the client know what we're up to.
	hr = precall->QueryInsertObject(&reobj.clsid, reobj.pstg,
			REO_CP_SELECTION);
	if( hr != NOERROR )
	{
		TRACEERRORSZ("QueryInsertObject() failed.");
		goto err;
	}

	hr = reobj.poleobj->SetClientSite(reobj.polesite);
	if(hr)
	{
		TRACEERRORSZ("SetClientSite() failed.");
		goto err;
	}

	if(hr = InsertObject(&reobj))
	{
		TRACEERRORSZ("InsertObject() failed.");
	}

err:
	if(reobj.poleobj)
		reobj.poleobj->Release();

	if(reobj.polesite)
		reobj.polesite->Release();

	if(reobj.pstg)
		reobj.pstg->Release();

	return hr;
}
#endif

// MAC doesn't have drop files.
#ifndef MACPORT

typedef void (WINAPI*DRAGFINISH)(HDROP);
typedef UINT (WINAPI*DRAGQUERYFILEA)(HDROP, UINT, LPSTR, UINT);
typedef UINT (WINAPI*DRAGQUERYFILEW)(HDROP, UINT, LPTSTR, UINT);
typedef BOOL (WINAPI*DRAGQUERYPOINT)(HDROP, LPPOINT);

LRESULT	CTxtEdit::OnDropFiles(HANDLE hDropFiles)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnDropFiles");

	UINT	cFiles;
	UINT	iFile;
	char	szFile[MAX_PATH];
	WCHAR	wFile[MAX_PATH];
	POINT	ptDrop;
	CTxtSelection * const psel = GetSel();
	HMODULE		hDLL = NULL;
	DRAGFINISH		fnDragFinish; 
	DRAGQUERYFILEA	fnDragQueryFileA;
	DRAGQUERYFILEW	fnDragQueryFileW;
	DRAGQUERYPOINT	fnDragQueryPoint;

	if (_fReadOnly)
		return 0;

	AssertSz((hDropFiles != NULL), "CTxtEdit::OnDropFiles invalid hDropFiles");

	// dynamic load Shell32

	hDLL = LoadLibraryA ("Shell32.DLL");
	if (hDLL)
	{
		fnDragFinish = (DRAGFINISH)GetProcAddress (hDLL, "DragFinish");
		fnDragQueryFileA = (DRAGQUERYFILEA)GetProcAddress (hDLL, "DragQueryFileA");
		fnDragQueryFileW = (DRAGQUERYFILEW)GetProcAddress (hDLL, "DragQueryFileW");
		fnDragQueryPoint = (DRAGQUERYPOINT)GetProcAddress (hDLL, "DragQueryPoint");
	}
	else
		return 0;

	if ( !fnDragFinish || !fnDragQueryFileA || !fnDragQueryFileW || !fnDragQueryPoint )
	{
		AssertSz(FALSE, "Shell32 GetProcAddress failed");
		goto EXIT0;
	}

	(*fnDragQueryPoint) ( (HDROP)hDropFiles, &ptDrop );
	if (VER_PLATFORM_WIN32_WINDOWS == dwPlatformId)
		cFiles = (*fnDragQueryFileA) ( (HDROP)hDropFiles, (UINT)-1, NULL, 0 );
	else
		cFiles = (*fnDragQueryFileW) ( (HDROP)hDropFiles, (UINT)-1, NULL, 0 );

	if ( cFiles )
	{
		POINT	ptl = {(LONG) ptDrop.x, (LONG) ptDrop.y};
		LONG	cp = 0;
		CRchTxtPtr  rtp(this);
		const CCharFormat	*pCF;		

		if (_pdp->CpFromPoint(ptl, NULL, &rtp, NULL, FALSE) >= 0)
		{
			cp = rtp.GetCp();
			pCF = rtp.GetCF();
		}
		else
		{
			cp = psel->GetCp();	
			pCF = GetCharFormat(psel->Get_iCF ());
		}
		
		// notify user for dropfile
		if (_dwEventMask & ENM_DROPFILES)
		{
			ENDROPFILES endropfiles;

			endropfiles.hDrop = hDropFiles;
			endropfiles.cp = cp;
			endropfiles.fProtected = !!(pCF->dwEffects & CFE_PROTECTED);

			if (0 != TxNotify( EN_DROPFILES, &endropfiles))
				// ignore drop file
				goto EXIT;
			
			// allow callback to update cp
			cp = endropfiles.cp;
		}

		psel->SetCp(cp);	
	}

	for ( iFile = 0;  iFile < cFiles; iFile++ )
	{
		if (VER_PLATFORM_WIN32_WINDOWS == dwPlatformId)
		{
			(*fnDragQueryFileA) ( (HDROP)hDropFiles, iFile, szFile, MAX_PATH );
			MultiByteToWideChar(CP_ACP, 0, szFile, -1, 
							wFile, MAX_PATH);
		}
		else
			(*fnDragQueryFileW) ( (HDROP)hDropFiles, iFile, wFile, MAX_PATH );

		InsertFromFile (wFile);
	}

EXIT:
	(*fnDragFinish) ( (HDROP)hDropFiles );

EXIT0:
	FreeLibrary (hDLL);
	return 0;
}
#else
LRESULT	CTxtEdit::OnDropFiles(HANDLE hDropFiles)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnDropFiles");

	return 0;
}
#endif	// MACPORT


///////////////////////////////  Exposable methods  //////////////////////////////////////


HRESULT	CTxtEdit::TxCharFromPos(LPPOINT ppt, LONG *plres)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxCharFromPos");

	if(!fInplaceActive())
	{
		// We have not valid display rectangle if this object is not active
		*plres = -1;
		return OLE_E_INVALIDRECT;
	}
	
	*plres = _pdp->CpFromPoint(*ppt, NULL, NULL, NULL, FALSE);

	return (*plres != -1) ? S_OK : E_FAIL;
}


HRESULT CTxtEdit::TxPosFromChar(LONG cp, POINT *ppt)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxPosFromChar");

	if(!fInplaceActive())
	{
		return OLE_E_INVALIDRECT;
	}

	CRchTxtPtr rtp(this, cp);

	if(_pdp->PointFromTp(rtp, NULL, FALSE, *ppt, NULL, TA_TOP) < 0)
		return E_FAIL;

	return S_OK;
}

HRESULT CTxtEdit::TxFindWordBreak(INT nFunction, LONG cp, LONG *plRet)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxFindWordBreak");

	CTxtPtr tp(this, cp);						// This validates cp
	LONG	cpSave = tp.GetCp();				// Save starting value
	
	if(!plRet)
		return E_INVALIDARG;
	
	*plRet = tp.FindWordBreak(nFunction);

	// WB_CLASSIFY and WB_ISDELIMITER return values; others return offsets
	// this function returns values, so it converts when necessary
	if(nFunction != WB_CLASSIFY && nFunction != WB_ISDELIMITER)
		*plRet += cpSave;
	
	return S_OK;
}

/*
 *	CTxtEdit::TxFindText (flags, cpMin, cpMost, pch, pcpRet)
 *
 *	@mfunc
 *		Find text in direction specified by flags starting at cpMin if
 *		forward search (flags & FR_DOWN nonzero) and cpMost if backward
 *		search.
 *
 *	@rdesc
 *		HRESULT (success) ? NOERROR : S_FALSE
 *
 *	@comm
 *		Caller is responsible for setting cpMin to the appropriate end of
 *		the selection depending on which way the search is proceding.
 */
HRESULT CTxtEdit::TxFindText(
	DWORD	flags,		//@parm Specify FR_DOWN, FR_MATCHCASE, FR_WHOLEWORD
	LONG	cpStart,	//@parm Find start cp
	LONG	cpLimit,	//@parm Find limit cp 
	TCHAR*	pch,		//@parm Null terminated string to search for
	LONG *	pcpRet)		//@parm Out parm to receive start of matched string
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxFindText");

	DWORD		cchText = GetTextLength();
	LONG		cchToFind;
	BOOL		fForward = flags & FR_DOWN;
	const BOOL	fSetCur = (cchText >= 4096);
	HCURSOR		hcur = NULL;				// Init to keep compiler happy

	// Validate parameters
	if (!pch || !pcpRet || !(cchToFind = wcslen(pch)) ||
		cpStart < 0 || cpLimit < -1)
	{
		if(pcpRet)
			*pcpRet = -1;
		return E_INVALIDARG;				// Nothing to search for
	}

	CTxtPtr	tp(this, cpStart);	  
	
	if(fSetCur)								// In case this takes a while...
		hcur = SetCursor(LoadCursor(0, IDC_WAIT));
	
	if(Get10Mode())							// RichEdit 1.0 only searches
		flags |= FR_DOWN;					//  forward

	*pcpRet = tp.FindText(cpLimit, flags, pch, cchToFind);

	if(fSetCur)
		SetCursor(hcur);
	
	return *pcpRet >= 0 ? NOERROR : S_FALSE;;
}

HRESULT CTxtEdit::TxGetLineCount(LONG *pcli)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxGetLineCount");

	AssertSz((pcli != NULL), "CTxtEdit::TxGetLineCount invalid pcli");

	if(!fInplaceActive())
	{
		return OLE_E_INVALIDRECT;
	}

	if(!_pdp->WaitForRecalc(GetTextLength(), -1))
	{
		return E_FAIL;
	}

	*pcli = _pdp->LineCount();
	Assert(*pcli > 0);

	return S_OK;
}


HRESULT CTxtEdit::TxLineFromCp(LONG cp, LONG *pli)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxLineFromCp");

	BOOL		fAtEnd = FALSE;

	AssertSz((pli != NULL), "CTxtEdit::TxLineFromCp invalid pli");

	if(!fInplaceActive())
	{
		AssertSz((*pli == 0), 
			"CTxtEdit::TxLineFromCp error return lres not correct");
		return OLE_E_INVALIDRECT;
	}

	if (cp < 0)									// Validate cp
	{
		if(_psel)
		{
			cp = _psel->GetCp();
			fAtEnd = _psel->CaretNotAtBOL();
		}
		else
		{
			cp = 0;
		}
	}
	else if (cp > (LONG)GetTextLength())
		cp = GetTextLength();
 
	*pli = _pdp->LineFromCp(cp, fAtEnd);

	
	HRESULT hr = *pli < 0 ? E_FAIL : S_OK;

	// Old messages expect 0 as a result of this call if there is an error.
	if (*pli == -1)
	{
		*pli = 0;
	}

	return hr;
}


HRESULT CTxtEdit::TxLineLength(LONG cp, LONG *pcch)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxLineLength");

	LONG	cch = 0;

	AssertSz((pcch != NULL), 
			"CTxtEdit::TxLineLength Invalid pcch parameter");

	if(!fInplaceActive())
	{
		return OLE_E_INVALIDRECT;
	}

	if(cp < 0)
	{
		if(!_psel)
			return E_FAIL;
		cch = _psel->LineLength();
	}
	else if(cp <= (LONG)GetAdjustedTextLength())
	{
		CLinePtr rp(_pdp);
		rp.RpSetCp(cp, FALSE);
		cch = rp.GetAdjustedLineLength();
	}
	*pcch = cch;
	return S_OK;
}

HRESULT CTxtEdit::TxLineIndex(LONG ili, LONG *pcp)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxLineIndex");

	HRESULT 	hresult;
	AssertSz((pcp != NULL), "CTxtEdit::TxLineIndex invalid pcp");

	*pcp = -1;

	if(!fInplaceActive())
	{
		return OLE_E_INVALIDRECT;
	}

	if( ili == -1 )
	{
		// fetch the line from the current cp.
		hresult = TxLineFromCp(-1, &ili);
		if( hresult != NOERROR )
		{
			return hresult;
		}
	}

	// ili is a zero-based *index*, whereas count returns the total # of lines.
	// therefore, we use >= for our comparisions.
	if( ili >= _pdp->LineCount() && !_pdp->WaitForRecalcIli(ili))
		return E_FAIL; 

	*pcp = _pdp->CpFromLine(ili, NULL);
	
	return S_OK;
}

///////////////////////////////////  Miscellaneous messages  ////////////////////////////////////


LRESULT CTxtEdit::OnFindText(UINT msg, DWORD flags, FINDTEXTEX *pftex)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnFindText");

	LRESULT lres;

	if (TxFindText(flags, pftex->chrg.cpMin, pftex->chrg.cpMost, pftex->lpstrText, &lres) != S_OK)
	{
		if( msg == EM_FINDTEXTEX )
		{
			pftex->chrgText.cpMin = -1;
			pftex->chrgText.cpMost = -1;
		}
		return -1;
	}

	if(msg == EM_FINDTEXTEX)					// we send a message back to the doc to change
	{											// the selection to this.
		pftex->chrgText.cpMin = lres;
		pftex->chrgText.cpMost = lres + wcslen(pftex->lpstrText);
	}
	
	return lres;
}
	
LRESULT CTxtEdit::OnGetWordBreakProc()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnGetWordBreakProc");

	LRESULT lres = FALSE;
#ifdef DBCS
	if(_pfnWB != (VEDITWORDBREAKPROC) IVWordBreakProc)
		lres = (LRESULT) LpVGetWordBreakProc(_lpPunctObj);
#else
	if(_pfnWB != (EDITWORDBREAKPROC) TxWordBreakProc)
		lres = (LRESULT) _pfnWB;
#endif
	return lres;
}

// For plain-text instances, OnGetCharFormat(), OnGetParaFormat(),
// OnSetCharFormat(), and OnSetParaFormat() apply to whole story

LRESULT CTxtEdit::OnGetCharFormat(CCharFormat *pf, DWORD flags)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnGetCharFormat");

	if(IsValidCharFormat(pf))
	{
		if( (flags & SCF_SELECTION) )
		{
			const CTxtRange * const psel = GetSel();
			Assert(psel);
			psel->GetCharFormat(pf, flags);
		}
		else
			GetCharFormat(-1)->Get(pf);
		if(pf->cbSize == sizeof(CHARFORMAT))		// Maintain CHARFORMAT
		{											//  compatibility
			pf->dwEffects &= CFM_EFFECTS;
			pf->dwMask &= CFM_ALL;
		}
		return pf->dwMask;
	}
	return 0;
}

LRESULT CTxtEdit::OnGetParaFormat(CParaFormat *pf)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnGetParaFormat");

	if(IsValidParaFormat(pf))
	{
		const CTxtRange * const psel = GetSel();
		Assert(psel);
		psel->GetParaFormat(pf);
		if(pf->cbSize == sizeof(PARAFORMAT))		// Maintain PARAFORMAT
			pf->dwMask &= PFM_ALL;					//  compatibility

		if((pf->rgxTabs[0] & PFT_DEFAULT) == PFT_DEFAULT)
			pf->rgxTabs[0] &= ~PFT_DEFAULT;			// Don't export default
		return pf->dwMask;							//  flag
	}
	return 0;
}

/*
 *	CTxtEdit::OnSetFont(hfont)
 *
 *	@mfunc
 *		Set new default font from hfont
 *
 *	@rdesc
 *		LRESULT nonzero if success
 */
LRESULT CTxtEdit::OnSetFont(
	HFONT hfont)			//@parm Handle of font to use for default
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnSetFont");

	CCharFormat	CF;

	if (FAILED(CF.InitDefault(hfont)) ||
		FAILED(OnSetCharFormat(0, (LPARAM)&CF, NULL)))
	{
		return 0;
	}
	return TRUE;
}

/*
 *	CTxtEdit::OnSetCharFormat(wparam, lparam, publdr)
 *
 *	@mfunc
 *		Set new default CCharFormat
 *
 *	@rdesc
 *		LRESULT nonzero if success
 */
LRESULT CTxtEdit::OnSetCharFormat(
	WPARAM		 wparam,	//@parm Selection flag
	LPARAM		 lparam,	//@parm Cast to CHARFORMAT *
	IUndoBuilder *publdr)	//@parm Undobuilder to receive antievents
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnSetCharFormat");

	if(!IsValidCharFormat((CHARFORMAT *)lparam))
	{
		AssertSz(0, "CTxtEdit::OnSetCharFormat():  Invalid CHARFORMAT structure");
		return 0;
	}

	BOOL fRet = TRUE;

	if(wparam & SCF_ALL) {
		CTxtRange rg(this, 0, -(LONG)GetTextLength());

		if(publdr)
		{
			publdr->StopGroupTyping();
		}

		fRet = rg.SetCharFormat((CCharFormat *)lparam, FALSE, publdr);

		// if we have an insertion point, apply the format to it as well
		if(_psel && _psel->GetCch() == 0)
		{
			if(!_psel->SetCharFormat((CCharFormat *)lparam, publdr, wparam))
			{
				fRet = FALSE;
			}
		}
	}
	else if( (wparam & SCF_SELECTION ))
	{
		// Change selection character format
		if(!_psel || IsProtected(EM_SETCHARFORMAT, wparam, lparam))
			return 0;

		return _psel->SetCharFormat((CCharFormat *)lparam, publdr, wparam) 
				== NOERROR;
	}

	// Change default character format

	CCharFormat			CF;						// Local CF to party on
	LONG				iCF;					// Possible new CF index
	const CCharFormat *	pCF;					// Ptr to current default CF
	ICharFormatCache  *	pICFCache;

	if (FAILED(GetCharFormatCache(&pICFCache)) ||
		FAILED(pICFCache->Deref(_iCF, &pCF)))	// Get ptr to current default
	{
		fRet = FALSE;							//  CCharFormat
		goto Update;
	}
	CF = *pCF;									// Copy current default CF
	CF.cbSize = sizeof(CHARFORMAT2);			// Restore cbSize
	CF.Apply((CCharFormat *)lparam, fInOurHost());			// Modify copy
	if (FAILED(pICFCache->Cache(&CF, &iCF)))	// Cache modified copy
	{
		fRet = FALSE;
		goto Update;
	}

	pICFCache->ReleaseFormat(_iCF);				// Release _iCF regardless
	_iCF = (WORD)iCF;							//  of whether _iCF = iCF,
												//  i.e., only 1 ref count

Update:
	// FUTURE (alexgo):  this may be unnecessary if the display handles
	// updating more automatically.
	_pdp->UpdateView();

	return fRet;
}

/*
 *	CTxtEdit::OnSetParaFormat(wparam, lparam, publdr)
 *
 *	@mfunc
 *		Set new default CParaFormat
 *
 *	@rdesc
 *		LRESULT nonzero if success
 */
LRESULT CTxtEdit::OnSetParaFormat(
	WPARAM		 wparam,	//@parm wparam passed thru to IsProtected()
	LPARAM		 lparam,	//@parm Cast to PARAFORMAT *
	IUndoBuilder *publdr)	//@parm Undobuilder to receive antievents
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnSetParaFormat");
	LRESULT result;

	if(!IsValidParaFormat((PARAFORMAT *)lparam))
		return 0;

	if(	!(wparam & SPF_SETDEFAULT))	
	{
		// if the DEFAULT flag is specified, don't change the selection
		if (!_psel || IsProtected(EM_SETPARAFORMAT, wparam, lparam))
			return 0;

		result = (_psel->SetParaFormat((CParaFormat *)lparam, publdr) 
						== NOERROR);

		// this is a bit funky, but basically, if the text is empty
		// then we also need to set the default paragraph format
		// (done in the code below).  Thus, if we hit a failure or
		// if the document is not empty, go ahead and return.  
		// Otherwise, fall through to the default case.

		if( !result || GetAdjustedTextLength() )
		{
			return result;
		}
	}

	// No text in document or (wparam & SCF_SETDEFAULT): set default format

	LONG				iPF;					// Possible new PF index
	CParaFormat			PF;						// Local PF to party on
	const CParaFormat *	pPF;					// Ptr to current default PF
	IParaFormatCache  *	pIPFCache;

	if (FAILED(GetParaFormatCache(&pIPFCache)) ||
		FAILED(pIPFCache->Deref(_iPF, &pPF)))	// Get ptr to current PF
	{
		return 0;
	}
	PF = *pPF;									// Copy current default PF
	PF.cbSize = sizeof(CParaFormat);			// Restore cbSize
	PF.Apply((CParaFormat *)lparam);			// Modify copy
	if(FAILED(pIPFCache->Cache(&PF, &iPF)))		// Cache modified copy
		return 0;
	pIPFCache->ReleaseFormat(_iPF);				// Release _iPF regardless of
	_iPF = (WORD)iPF;							// Update default format index
	_pdp->UpdateView();
	return TRUE;
}


////////////////////////////////  System notifications  ////////////////////////////////

LRESULT CTxtEdit::OnSetFocus()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnSetFocus");

	_fFocus = TRUE;

	if( !_psel )
	{
		return 0;
	}

	// _fMouseDown may sometimes be true.
	//this can happen when somebody steals our focus when we were doing
	//something with the mouse down--like processing a click. Thus, we'll
	//never get the MouseUpMessage.
#ifdef DEBUG
	if( _fMouseDown )
	{
		TRACEWARNSZ("Getting the focus, yet we think the mouse is down");
	}
#endif // DEBUG
	_fMouseDown = FALSE;



	_psel->UpdateCaret(_fScrollCaretOnFocus);
	_fScrollCaretOnFocus = FALSE;

	_psel->ShowSelection(TRUE);
	
	TxNotify(EN_SETFOCUS, NULL);

	return 0;
}


LRESULT CTxtEdit::OnKillFocus()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnKillFocus");

	if( _pundo )
	{
		_pundo->StopGroupTyping();
	}

	if(_fHideSelection && _psel)
		_psel->ShowSelection(FALSE);

	_fFocus = FALSE;

	DestroyCaret();
	
	TxNotify(EN_KILLFOCUS, NULL);

	_fScrollCaretOnFocus = FALSE;		// just to be safe, clear this
	return 0;
}


#ifdef	DEBUG
void CTxtEdit::OnDumpPed()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnDumpPed");

	char sz[256];
	CTxtSelection * const psel = GetSel();
	SELCHANGE selchg;

	psel->SetSelectionInfo(&selchg);

	wsprintfA(sz,
		"cchText = %ld		cchTextMost = %ld\r\n"
		"cpSelActive = %ld		cchSel = %ld\r\n"
		"wSelType = %x		# lines = %ld",
		GetTextLength(),	TxGetMaxLength(),
		psel->GetCp(),	psel->GetCch(),
		selchg.seltyp,	_pdp->LineCount()
	);
	Tracef(TRCSEVINFO, "%s", sz);
	MessageBoxA(0, sz, "ED", MB_OK);
}
#endif					// DEBUG



///////////////////////////// Scrolling Commands //////////////////////////////////////


HRESULT CTxtEdit::TxHScroll(WORD wCode, int xPos)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxHScroll");

	if (!fInplaceActive())
	{
		return OLE_E_INVALIDRECT;
	}

	_pdp->HScroll(wCode, xPos);

	return S_OK;
}


LRESULT CTxtEdit::TxVScroll(WORD wCode, int yPos)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxVScroll");

	return _pdp->VScroll(wCode, yPos);
}


HRESULT CTxtEdit::TxLineScroll(LONG cli, LONG cch)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxLineScroll");

	_pdp->LineScroll(cli, cch);
	return S_OK;
}


void CTxtEdit::OnScrollCaret()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnScrollCaret");

	if(_psel)
		_psel->UpdateCaret(TRUE);
}




///////////////////////////////// Editing messages /////////////////////////////////


void CTxtEdit::OnClear( IUndoBuilder *publdr)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnClear");

	if(!_psel || TxGetReadOnly())
	{
		Sound();
		return;
	}
	
	if(_psel->GetCch() != 0)
	{
		if(!IsProtected(WM_CLEAR, 0, 0))
			_psel->ReplaceRange(0, NULL, publdr, SELRR_REMEMBERRANGE);
	}
}

LONG CTxtEdit::OnReplaceSel(LONG cchNew, const TCHAR *pch, IUndoBuilder *publdr)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::OnReplaceSel");

	if(!GetSel())
	{
		Sound();
		return 0;
	}
	
	// If we aren't given an undo builder, then clear the undo stack.
	if( !publdr )
	{
		if( GetUndoMgr() )
		{
			GetUndoMgr()->ClearAll();
		}
		if( GetRedoMgr() )
		{
			GetRedoMgr()->ClearAll();
		}
	}
	else
	{
		publdr->StopGroupTyping();
	}

	return _psel->CleanseAndReplaceRange(cchNew, pch, FALSE, publdr);
}

void CTxtEdit::Sound()
{
	if(_fAllowBeep)
		Beep();
}


///////////////////////////////  Memory mamagement  /////////////////////////////////

/*
 *	PvAllocFn (cbBuf, uiMemFlags)
 *
 *	@mfunc	memory allocation.  Similar to GlobalAlloc, but uses
 *			OLE's memory allocator
 *
 *	@comm	The only flag of interest is GMEM_ZEROINIT, which
 *			specifies that memory should be zeroed after allocation.
 */
LPVOID PvAlloc(
	ULONG	cbBuf, 			//@parm	Count of bytes to allocate
	UINT	uiMemFlags)		//@parm Flags controlling allocation
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "PvAlloc");
	
	void * pv = CoTaskMemAlloc(cbBuf);

	if( pv && (uiMemFlags & GMEM_ZEROINIT) )
	{
		#if defined(DEBUG) && !defined(MACPORT) && !defined(_ALPHA_)
		TrackBlock(pv);
		#endif
		ZeroMemory(pv, cbBuf);
	}
	
	return pv;
}

/*
 *	PvReAllocFn	(pvBuf, cbBuf)
 *
 *	@mfunc	memory reallocation.  Uses OLE's memory allocator
 *
 *	FUTURE	(alexgo) this should be inline if we don't add any extra
 *			code here (like to zero the memory)
 */
LPVOID PvReAlloc(
	LPVOID	pvBuf, 		//@parm Buffer to reallocate
	DWORD	cbBuf)		//@parm New size of buffer
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "PvReAlloc");

	return CoTaskMemRealloc(pvBuf, cbBuf);
}

/*
 *	FreePvFn (pvBuf)
 *
 *	@mfunc	free's memory using OLE's memory allocator
 *
 *	@rdesc	TRUE if pvBuf is not NULL
 */
BOOL FreePv(
	LPVOID pvBuf)		//@parm Buffer to free
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "FreePv");

	if( pvBuf )
	{
		CoTaskMemFree(pvBuf);
		return TRUE;
	}
	return FALSE;
}


void* _cdecl operator new (size_t size)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "new");

	return PvAlloc(size, GMEM_ZEROINIT);
}

void _cdecl operator delete (void* P)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "delete");

	if(P)
		FreePv(P);
}


///////////////////////////////////  Miscellaneous  ///////////////////////////////////////////

void CTxtEdit::TxGetViewInset(LPRECT prc, CDisplay *pdp) const
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxGetViewInset");

	
	// Get the inset which is in HIMETRIC
	RECT rcHiMetricViewInset;

	if (SUCCEEDED(_phost->TxGetViewInset(&rcHiMetricViewInset)))
	{
		if (NULL == pdp)
		{
			// Default to the main display if no display specified.
			pdp = _pdp;
		}

		AssertSz(pdp->IsValid(), "CTxtEdit::TxGetViewInset Device not valid");

		// Convert the HIMETRIC to pixels
		prc->left = pdp->HimetricXtoDX(rcHiMetricViewInset.left);
		prc->top =  pdp->HimetricYtoDY(rcHiMetricViewInset.top);
		prc->right =  pdp->HimetricXtoDX(rcHiMetricViewInset.right);
		prc->bottom =  pdp->HimetricYtoDY(rcHiMetricViewInset.bottom);

		// Get the zoom factor information
		LONG lZoomNumerator = pdp->GetZoomNumerator();
		LONG lZoomDenominator = pdp->GetZoomDenominator();

		// If there is a zoom factor, apply it. Note: since the window's
		// host never zooms, this test saves it a fair amount of useless work.
		if (lZoomNumerator != lZoomDenominator)
		{
			prc->left = MulDiv(prc->left, lZoomNumerator, lZoomDenominator);
			prc->top = MulDiv(prc->top, lZoomNumerator, lZoomDenominator);
			prc->right = MulDiv(prc->right, lZoomNumerator, lZoomDenominator);
			prc->bottom = MulDiv(prc->bottom, lZoomNumerator, lZoomDenominator);
		}
	}
	else
	{
		// The call to the host failed. While this is highly improbable, we do 
		// want to something reasonably sensible. Therefore, we will just pretend 
		// there is no inset and continue.
		ZeroMemory(prc, sizeof(RECT));
	}
}

// Interchange horizontal and vertical commands
WORD wConvScroll(WORD wparam)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "wConvScroll");

	switch(wparam)
	{
		case SB_BOTTOM:
			return SB_TOP;

		case SB_LINEDOWN:
			return SB_LINEUP;

		case SB_LINEUP:
			return SB_LINEDOWN;

		case SB_PAGEDOWN:
			return SB_PAGEUP;

		case SB_PAGEUP:
			return SB_PAGEDOWN;

		case SB_TOP:
			return SB_BOTTOM;

		default:
			return wparam;
	}
}

//
//	helper functions. FUTURE (alexgo) maybe we should get rid of
//  some of these
//

/*	FUTURE (murrays): Unless they are called a lot, the TxGetBit routines
	might be done more compactly as:

BOOL CTxtEdit::TxGetBit(
	DWORD dwMask)
{
	DWORD dwBits = 0;
	_phost->TxGetPropertyBits(dwMask, &dwBits);
	return dwBits != 0;
}

e.g., instead of TxGetSelectionBar(), we use TxGetBit(TXTBIT_SELECTIONBAR).
If they are called a lot (like TxGetSelectionBar()), the bits should probably
be cached, since that saves a bunch of cache misses incurred in going over to
the host.

*/

TXTBACKSTYLE CTxtEdit::TxGetBackStyle() const					
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxGetBackStyle");

	TXTBACKSTYLE style = TXTBACK_OPAQUE;
	_phost->TxGetBackStyle(&style);
	return style;
}

BOOL CTxtEdit::TxGetAutoSize() const					
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxGetAutoSize");

	return (_dwEventMask & ENM_REQUESTRESIZE);
}

BOOL CTxtEdit::TxGetAutoWordSel() const				
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxGetAutoWordSel");

	DWORD dwBits = 0;
	_phost->TxGetPropertyBits(TXTBIT_AUTOWORDSEL, &dwBits);
	return dwBits != 0;
}

DWORD CTxtEdit::TxGetMaxLength() const					
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxGetMaxLength");

	return _cchTextMost;
}

/*
 *	CTxtEdit::TxSetMaxToMaxText()
 *
 *	@mfunc
 *		Set new maximum text length based on length of text
 *
 */
void CTxtEdit::TxSetMaxToMaxText()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxSetMaxToMaxText");

	// See if we need to update the text max
	LONG cchRealLen = GetAdjustedTextLength();

	if (_fInOurHost && _cchTextMost < cchRealLen)
	{
		_cchTextMost = cchRealLen;
	}		
}

BOOL CTxtEdit::TxGetMultiLine() const					
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxGetMultiLine");

	DWORD dwBits = 0;
	_phost->TxGetPropertyBits(TXTBIT_MULTILINE, &dwBits);
	return dwBits != 0;
}

TCHAR CTxtEdit::TxGetPasswordChar() const				
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxGetPasswordChar");

	if (_fUsePassword)
	{
		TCHAR ch = '*';
		_phost->TxGetPasswordChar(&ch);
		return ch;
	}
	return 0;
}

DWORD CTxtEdit::TxGetScrollBars() const					
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxGetScrollBars");

	DWORD dwScroll;
	_phost->TxGetScrollBars(&dwScroll);
	return dwScroll;
}

BOOL CTxtEdit::TxGetSelectionBar() const				
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxGetSelectionBar");
	
	LONG lSelBarWidth;
	_phost->TxGetSelectionBarWidth(&lSelBarWidth);
	return lSelBarWidth != 0;
}

BOOL CTxtEdit::TxGetWordWrap(void) const
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxGetWordWrap");

	DWORD dwBits = 0;
	_phost->TxGetPropertyBits(TXTBIT_WORDWRAP, &dwBits);
	return dwBits != 0;
}

BOOL CTxtEdit::TxGetSaveSelection(void) const
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::TxGetSaveSelection");

	DWORD dwBits = 0;
	_phost->TxGetPropertyBits(TXTBIT_SAVESELECTION, &dwBits);
	return dwBits != 0;
}


/* 
 *	CTxtEdit::ClearUndo()
 *
 *	@mfunc	Clear all undo buffers
 */
void CTxtEdit::ClearUndo(
	IUndoBuilder *publdr)	//@parm the current undo context (may be NULL)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::ClearUndo");

	if( _pundo )
	{
		_pundo->ClearAll();
	}

	if( _predo )
	{
		_predo->ClearAll();
	}

	if( publdr )
	{
		publdr->Discard();
	}
}

/*
 *	CTxtEdit::TxIsDoubleClickPending
 *
 *	@mfunc	calls to the host on ITextHost2
 *
 *	@rdesc 	TRUE/FALSE
 */
BOOL CTxtEdit::TxIsDoubleClickPending()
{
	ITextHost2 *phost2;
	BOOL	fPending;

	if( _phost->QueryInterface(IID_ITextHost2, (void **)&phost2) 
		== NOERROR )
	{
		fPending = phost2->TxIsDoubleClickPending();
		phost2->Release();
		return fPending;
	}
	return FALSE;
}			

/*
 *	CTxtEdit::TxGetWindow
 *
 *	@mfunc	calls on the host via ITextHost2 to get the current
 *			window for this edit instance.  This is very helpful for
 *			OLE object support
 *
 *	@rdesc	HRESULT
 */
HRESULT	CTxtEdit::TxGetWindow(HWND *phwnd)
{
	ITextHost2 *phost2;
	HRESULT hr;

	if( _phost->QueryInterface(IID_ITextHost2, (void **)&phost2) 
		== NOERROR )
	{
		hr = phost2->TxGetWindow(phwnd);
		phost2->Release();
		return hr;
	}
	return E_NOINTERFACE;
}

/*
 *	CTxtEdit::TxSetForegroundWindow
 *
 *	@mfunc	calls on the host via ITextHost2 to set our window
 *			to the foreground window. Used to support drag/drop.
 *
 *	@rdesc	HRESULT
 */
HRESULT	CTxtEdit::TxSetForegroundWindow()
{
	ITextHost2 *phost2;
	HRESULT hr;

	if( _phost->QueryInterface(IID_ITextHost2, (void **)&phost2) 
		== NOERROR )
	{
		hr = phost2->TxSetForegroundWindow();
		phost2->Release();
		return hr;
	}

	return E_NOINTERFACE;
}

/*
 *	CTxtEdit::TxGetPalette
 *
 *	@mfunc	calls on the host via ITextHost2 to set our window
 *			to the foreground window. Used to support drag/drop.
 *
 *	@rdesc	HRESULT
 */
HPALETTE CTxtEdit::TxGetPalette()
{
	ITextHost2 *phost2;
	HPALETTE hpal = NULL;

	if( _phost->QueryInterface(IID_ITextHost2, (void **)&phost2) 
		== NOERROR )
	{
		hpal = phost2->TxGetPalette();
		phost2->Release();
	}

	return hpal;
}

//
//	Event Notification methods
//

/*
 *	CTxtEdit::TxNotify(iNotify, pv)
 *
 *	@mfunc	This function checks bit masks and sends notifications to the
 *			host.
 *
 *	@devnote	Callers should check to see if a special purpose notification
 *			method has already been provided.
 *
 *	@rdesc	S_OK, S_FALSE, or some error
 */
HRESULT CTxtEdit::TxNotify( 
	DWORD iNotify, 		//@parm Notification to send
	void *pv )			//@parm Data associated with notification
{
	// First, disallow notifications that we handle elsewhere
	Assert(iNotify != EN_SELCHANGE); 	//see SetSelectionChanged
	Assert(iNotify != EN_ERRSPACE);		//see SetOutOfMemory
	Assert(iNotify != EN_CHANGE);		//see SetChangedEvent
	Assert(iNotify != EN_HSCROLL);		//see SendScrollEvent
	Assert(iNotify != EN_VSCROLL);		//see SendScrollEvent
	Assert(iNotify != EN_MAXTEXT);		//see SetMaxText
	Assert(iNotify != EN_MSGFILTER);	//this is handled specially
										// in TxSendMessage

	// switch on the event to check masks.  

	switch( iNotify )
	{
		case EN_DROPFILES:
			if( (_dwEventMask & ENM_DROPFILES) )
			{
				goto Notify;
			}
			break;
		case EN_PROTECTED:
			if( (_dwEventMask & ENM_PROTECTED) )
			{
				goto Notify;
			}
			break;
		case EN_REQUESTRESIZE:
			if( (_dwEventMask & ENM_REQUESTRESIZE) )
			{
				goto Notify;
			}
			break;
		Notify:
		default:
			return _phost->TxNotify(iNotify, pv);
	}
	return NOERROR;
}
			
/*
 *	CTxtEdit::SendScrollEvent(iNotify)
 *
 *	@mfunc	sends the scroll event if appropriate
 *
 *	@comm	Scroll events must be sent before any view updates have
 *			been requested and only if ENM_SCROLL is set.
 */
void CTxtEdit::SendScrollEvent(
	DWORD iNotify)		//@parm the notification to send
{
	Assert(iNotify == EN_HSCROLL || iNotify == EN_VSCROLL);

	// FUTURE (alexgo/ricksa).  The display code can't really
	// handle this assert yet.  Basically, we're trying to
	// say that scrollbar notifications have to happen
	// _before_ the window is updated.  When we do the
	// display rewrite, try to handle this better.

	// Assert(_fUpdateRequested == FALSE);

	_phost->TxNotify(iNotify, NULL);
	
	return;
}

/*
 *	CTxtEdit::HandleLinkNotification
 *
 *	@mfunc	Handles sending EN_LINK notifications.
 *
 *	#rdesc	TRUE if the EN_LINK message was sent and 
 *			procesed successfully.  Typically, that means the
 *			caller should stop whatever processing it was doing.
 */
BOOL CTxtEdit::HandleLinkNotification(
	UINT msg,			//@parm	the msg prompting the link notification
	WPARAM wparam,		//@parm the wparam of the message
	LPARAM lparam,		//@parm the lparam of the message
	BOOL *pfInLink)		//@parm if non-NULL, indicate if over a link
{
	if( pfInLink )
	{
		*pfInLink = FALSE;
	}

	if( (_dwEventMask & ENM_LINK) && _fInPlaceActive)
	{
		LONG cp;
		POINT pt;


		if( msg != WM_SETCURSOR )
		{
			pt.x = LOWORD(lparam);
			pt.y = HIWORD(lparam);
		}
		else
		{
			GetCursorPos(&pt);
			if( !_phost->TxScreenToClient(&pt) )
			{
				return FALSE;
			}
		}

		cp = _pdp->CpFromPoint(pt, NULL, NULL, NULL, FALSE); 

		if( cp >= 0 )
		{
			CTxtRange rg(this, cp, 0);
			ENLINK enlink;
			LONG cpMin, cpMost;

			if( rg.GetCF()->dwEffects & CFE_LINK )
			{
				if( pfInLink )
				{
					*pfInLink = TRUE;
				}

				//hit a link, now fill in the data structures
				//for our EN_LINK callback and ask what we 
				//should do.
								
				rg.ExpandToLink();

				rg.GetRange(cpMin, cpMost);

				enlink.msg = msg;
				enlink.wParam = wparam;
				enlink.lParam = lparam;
				enlink.chrg.cpMin = cpMin;
				enlink.chrg.cpMost = cpMost;

				if( _phost->TxNotify(EN_LINK, &enlink) == S_FALSE )
				{
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

/*
 *	CTxtEdit::QueryUseProtection(prg, msg, wparam, lparam)
 *
 *	@mfunc	sends EN_PROTECTED to the host, asking if we should continue
 *	to honor the protection on a given range of characters
 *
 *	@rdesc	TRUE if protection should be honored, FALSE otherwise
 */
BOOL CTxtEdit::QueryUseProtection(
	CTxtRange *prg,	 	//@parm range to check for
	UINT	msg,   		//@parm msg used
	WPARAM	wparam,		//@parm wparam of the msg
	LPARAM 	lparam)		//@parm lparam of the msg
{
	ENPROTECTED enp;
	LONG	cpMin, cpMost;
	CCallMgr *pcallmgr = GetCallMgr();
	BOOL fRet = FALSE;

	Assert((_dwEventMask & ENM_PROTECTED));
	

	if( pcallmgr->GetInProtected() )
	{
		return FALSE;
	}

	pcallmgr->SetInProtected(TRUE);

	ZeroMemory(&enp, sizeof(ENPROTECTED));
	
	prg->GetRange(cpMin, cpMost);

	enp.msg = msg;
	enp.wParam = wparam;
	enp.lParam = lparam;
	enp.chrg.cpMin = cpMin;
	enp.chrg.cpMost = cpMost;

	if( _phost->TxNotify(EN_PROTECTED, &enp) == S_FALSE )
	{
		fRet = TRUE;
	}

	pcallmgr->SetInProtected(FALSE);

	return fRet;
}

/*
 *	CTxtEdit::TxReleaseMeasureDC(hMeasureDC)
 *
 *	@mfunc	Release the measure DC so we can get from TxGetMeasureDC()
 *
 *	@rdesc	
 *		1 - HDC was released. <nl>
 *		0 - HDC was not released. <nl> 
 */
int CTxtEdit::TxReleaseMeasureDC(
	HDC		hMeasureDC)	 	//@parm DC to be released
{
	if (fInplaceActive())
		return GetHost()->TxReleaseDC(hMeasureDC);
	else
		// delete the IC that we have created
		return ::DeleteDC (hMeasureDC);
}

#ifdef DEBUG
//This is a debug api used to dump the document runs.
//If a pointer to the ped is passed, it is saved and
//used.  If NULL is passed, the previously saved ped
//pointer is used.  This allows the "context" to be
//setup by a function that has access to the ped and
//DumpDoc can be called lower down in a function that
//does not have access to the ped.
extern "C" {
void DumpStory(void * ped)
{
    static CTxtEdit * pedSave = NULL;

    if (NULL != ped)
       pedSave = (CTxtEdit*)ped;

    if (NULL != pedSave)
    {
        CTxtStory * pStory = pedSave->GetTxtStory();
        if (NULL != pStory)
            pStory->DbgDumpStory();
		CObjectMgr * pobjmgr = pedSave->GetObjectMgr();
        if (NULL != pobjmgr)
            pobjmgr->DbgDump();
    }

    return;
}
}
#endif

/*
 *	CTxtEdit::CheckUnicode(lStreamFormat)
 *
 *	@mfunc
 *		If lStreamFormat has the Unicode bit set, store the Unicode
 *		code page 1200 in _pDocInfo->uCpg
 */
void CTxtEdit::CheckUnicode(
	LONG		lStreamFormat)	// @parm stream format to use for loading
{
	if(lStreamFormat & SF_UNICODE)
	{
		if(!_pDocInfo)
			_pDocInfo = new CDocInfo();
		if(_pDocInfo)
			_pDocInfo->wCpg = 1200;			// Store Unicode code page
	}
}

/*
 *	CTxtEdit::TxGetDefaultCharFormat
 *
 *	@mfunc	helper function to retrieve character formats from the
 *			host.  Does relevant argument checking
 *
 *	@rdesc	HRESULT
 */
HRESULT CTxtEdit::TxGetDefaultCharFormat(
	CCharFormat *pCCF)		//@parm character format to fill in
{
	const CHARFORMAT *pcf = NULL;

	if( _phost->TxGetCharFormat(&pcf) == NOERROR )
	{
		if( IsValidCharFormat(pcf) )
		{
			if( (pcf->dwMask == CFM_ALL) ||
				(pcf->dwMask == CFM_ALL2 ) ||
				(pcf->dwMask == (CFM_ALL2 & ~CFM_BOLD)) ||
				(pcf->dwMask == (CFM_ALL2 & ~CFM_WEIGHT)) ||
				// this last check is incredibly lame, but necessary
				// to satisfy Forms^3.  The plan was for them to use 
				// CharForamt2, but shortly before shipping they 
				// decided to use CharFormat1 + the disabled bit.
				(pcf->dwMask == CFM_ALL | CFM_DISABLED) )
			{
				// the mask is valid as long as it specifies
				// everything.  Note that CFM_BOLD and CFM_WEIGHT
				// are mutually exclusive.  We don't catch
				// the case where 2.0 masks are mixed with
				// 1.0 data, but that is handled gracefully elsewhere.

				CopyMemory(pCCF, pcf, pcf->cbSize);
			}
			else
			{
				// use a default format and apply the one coming in.				
				TRACEWARNSZ("incomplete charformat mask from client; merging with default");

				pCCF->InitDefault(NULL);

				// ensure that all CCharFormat manipulated internally are
				// not CHARFORMAT's or CHARFORMAT2's
				CCharFormat cfInt;

				CopyMemory(&cfInt, pcf, pcf->cbSize);

				pCCF->Apply(&cfInt, fInOurHost());
			}
			return NOERROR;
		}
		TRACEERRORSZ("!!!!!! Attempting to recover from bad charformat...");

	}

	return pCCF->InitDefault(0);
}

/*
 *	CTxtEdit::TxGetDefaultParaFormat
 *
 *	@mfunc	helper function to retrieve  paragraph formats.  Does
 *			the relevant argument checking.
 *
 *	@rdesc	HRESULT
 */
HRESULT CTxtEdit::TxGetDefaultParaFormat(
	CParaFormat *pCPF)		//@parm paragraph format to fill in
{
	const PARAFORMAT *pPF = NULL;

	if( _phost->TxGetParaFormat(&pPF) == NOERROR)
	{
		if( IsValidParaFormat(pPF) )
		{
			if( pPF->dwMask == PFM_ALL || pPF->dwMask == PFM_ALL2 )
			{
				CopyMemory(pCPF, pPF, pPF->cbSize);
				return NOERROR;
			}
			else
			{
				// the check above will miss the case where
				// a paraformat1 was given paraformat2 masks.
				// Since those masks will be ignored, don't bother
				// with a check
				PARAFORMAT2 PFtemp;

				TRACEWARNSZ("incomplete charformat mask from client; merging with default");

				CopyMemory(&PFtemp, pPF, pPF->cbSize);

				pCPF->InitDefault();
				pCPF->Apply((CParaFormat *)&PFtemp);

				return NOERROR;
			}
		}
		TRACEERRORSZ("!!!!!! Attempting to recover from bad paraformat...");
	}

	return pCPF->InitDefault();
}



/*
 *	CTxtEdit::CreateMeasureDC
 *
 *	@mfunc	Set up metafile DC for rendering and create a DC that
 *			we can use for measuring for the metafile.
 *
 *	@rdesc	HDC of measure DC created
 *
 *	@devnote
 *			If the fUseTwips flag is TRUE, all the xWindow parameters
 *			are ignored for purposes of figuring out logical mapping.
 *			The reason for this is to make the EM_FORMATRANGE
 *			stuff work the way it did in 1.0 for metafiles.
 */
HDC CTxtEdit::CreateMeasureDC(
	HDC hdcMetaFile,	//@parm Metafile DC
	const RECT *prcClient, //@parm Client rectangle (optional).
	BOOL fUseTwips,		//@parm Whether we should use a TWIPs mapping
	LONG xWindowOrg,	//@parm x-Window origin
	LONG yWindowOrg,	//@parm y-Window origin
	LONG xWindowExt,	//@parm x-Window extent
	LONG yWindowExt,	//@parm y-Window extent
	LONG *pxPerInch,	//@parm x per inch for measure device
	LONG *pyPerInch)	//@parm y per inch for measure device
{
	RECT rcClient;

	// Set up the metafile HDC
	SetWindowOrgEx(hdcMetaFile, xWindowOrg, yWindowOrg, NULL);
	SetWindowExtEx(hdcMetaFile, xWindowExt, yWindowExt, NULL);

	// Get an HDC for the screen
	BOOL fActive = fInplaceActive();
	HDC hdcMeasure = fActive
		? GetHost()->TxGetDC()
		: CreateIC (TEXT("DISPLAY"), NULL, NULL, NULL);

	if (NULL == hdcMeasure)
	{
		return NULL;
	}

	if (fUseTwips)
	{
		SetMapMode(hdcMeasure, MM_TWIPS);
		*pyPerInch = LY_PER_INCH;
		*pxPerInch = LX_PER_INCH;
		return hdcMeasure;
	}
	
	LONG xPerInchDev = GetDeviceCaps(hdcMeasure, LOGPIXELSX);
	LONG yPerInchDev = GetDeviceCaps(hdcMeasure, LOGPIXELSY);

	if (NULL == prcClient)
	{
		// We can get the client rectangle to determine the size
		prcClient = &rcClient;

		TxGetClientRect(&rcClient);
	}

	// Get our extent size and convert that to pixels
	SIZEL sizelExtent;
	TxGetExtent(&sizelExtent);

	LONG yHeight = prcClient->bottom - prcClient->top;
	LONG xWidth = prcClient->right - prcClient->left;

	*pyPerInch = MulDiv(yHeight, 2540, sizelExtent.cy);
	*pxPerInch = MulDiv(xWidth, 2540, sizelExtent.cx);
		 
	// Set up the HDC for the screen to reflect the metafile measuring
	// paramters.
	SetMapMode(hdcMeasure, MM_ANISOTROPIC);
	SetWindowOrgEx(hdcMeasure, xWindowOrg, yWindowOrg, NULL);
	SetWindowExtEx(hdcMeasure, xWindowExt, yWindowExt, NULL);
	SetViewportExtEx(hdcMeasure, MulDiv(xWindowExt, xPerInchDev, *pxPerInch), 
		MulDiv(yWindowExt, yPerInchDev, *pyPerInch), NULL);
	return hdcMeasure;
}

/*
 *	CTxtEdit::GetAdjustedTextLength
 *
 *	@mfunc	retrieves the text length adjusted for the
 *			default end-of-document marker
 *
 *	@rdesc	DWORD
 *
 *	@devnote:	For Word and RichEdit compatibility, we insert a
 *			CR or CRLF at the end of every new document.  This routine
 *			calculates the size of the document _without_ the
 *			extra EOD marker.
 *
 *			For 1.0 compatibility, we insert a CRLF.  However, TOM
 *			requires that we use a CR, so 2.0 does that instead.
 */
DWORD CTxtEdit::GetAdjustedTextLength()
{
	DWORD length = GetTextLength();

	if( Get10Mode() )
	{
		Assert(IsRich());			// no plain text controls in 
									// 1.0 mode.
		length -= CCH_EOD_10;		// subtract the ending CRLF
	}
	else if( IsRich() )
	{
		Assert(length >= CCH_EOD_20);
		length -= CCH_EOD_20;		// subtract the ending CR
	}

	return length;
}

/*
 *	CTxtEdit::Set10Mode
 *
 *	@mfunc	Turns on the 1.0 compatibility mode bit.  However, if we
 *			are rich text, then we've already added a default 'CR' to
 *			the end-of-document.  We'll need to turn this into a 
 *			a CRLF for compatibility with RichEdit 1.0.
 *
 *	@rdesc	void
 *
 *	@devnote	this function should only be called _immediately_ after
 *			creation of text services and before all other work.  There
 *			are asserts to help ensure this.
 */
void CTxtEdit::Set10Mode()
{
	CCallMgr	callmgr(this);
	_f10Mode = TRUE;

	// make sure nothing important has happened to the control.
	// If these values are non-NULL, then somebody is probably trying
	// to put us into 1.0 mode after we've already done work as
	// a 2.0 control.	
	Assert(GetTextLength() == cchCR);
	Assert(_psel == NULL );
	Assert(_fModified == NULL);

	SetRichDocEndEOP(cchCR);

	if( !_pundo )
	{
		CreateUndoMgr(1, US_UNDO);
	}

	if( _pundo )
	{
		((CUndoStack *)_pundo)->EnableSingleLevelMode();
	}

	return;
}


/*
 *	CTxtEdit::SetRichDocEndEOP
 *
 *	@mfunc	Place the automatic EOP at the end of a rich text document.
 *
 *	@rdesc	void
 *
 */
void CTxtEdit::SetRichDocEndEOP(LONG cchToReplace)
{
	CRchTxtPtr rtp(this, 0);

	// Assume this is a 2.0 Doc
	LONG cchEOP = cchCR;
	WCHAR *pszEOP = szCR;

	if (_f10Mode)
	{
		// Reset the update values for a 1.0 doc
		cchEOP = cchCRLF;
		pszEOP = szCRLF;
	}

	rtp.ReplaceRange(cchToReplace, cchEOP, pszEOP, NULL, -1);
	
	_fModified = FALSE;
	_fSaved = TRUE;
	GetCallMgr()->ClearChangeEvent();

	return;
}



/*
 *	CTxtEdit::PopAndExecuteAntiEvent
 *
 *	@mfunc	Freeze display and execute anti-event
 *
 *	@rdesc	HRESULT from IUndoMgr::PopAndExecuteAntiEvent
 *
 */
HRESULT	CTxtEdit::PopAndExecuteAntiEvent(
	IUndoMgr *pundomgr,		//@parm undo manager to direct call to
	DWORD dwDoToCookie)		//@parm cookie for the undo manager
{
	HRESULT hr;
	// let the stack based classes clean up before restoring the 
	// selection
	{
		CFreezeDisplay fd(_pdp);
		CSelPhaseAdjuster	selpa(this);

		hr = pundomgr->PopAndExecuteAntiEvent(dwDoToCookie);
	}

	if( _psel )
	{
		// once undo/redo has been executed, flush the insertion
		// point formatting.
		_psel->Update_iFormat(-1);
		_psel->Update(TRUE);
	}

	return hr;
}

/*
 *	CTxtEdit::PasteDataObjectToRange
 *
 *	@mfunc	Freeze display and paste object
 *
 *	@rdesc	HRESULT from IDataTransferEngine::PasteDataObjectToRange
 *
 */
HRESULT	CTxtEdit::PasteDataObjectToRange( 
	IDataObject *pdo, 
	CTxtRange *prg, 
	CLIPFORMAT cf, 
	REPASTESPECIAL *rps,
	IUndoBuilder *publdr, 
	DWORD dwFlags )
{
	HRESULT hr = _ldte.PasteDataObjectToRange(pdo, prg, cf, rps, publdr, 
		dwFlags);

	if (_psel)
	{
		_psel->Update(TRUE);		   // now update the caret
	}

	return hr;
}

/*
 *	GetECDefaultHeightAndWidth
 *
 *	@mfunc	Helper for host to get ave char width and height for default character set
 *			for the control.
 *
 *	@rdesc	Height of default character set
 *
 *	@devnote:
 *			This really only s/b called by the window's host.
 *
 */
LONG GetECDefaultHeightAndWidth(
	ITextServices *pts,			//@parm ITextServices to conver to CTxtEdit.
	HDC hdc,					//@parm DC to use for retrieving the font.
	LONG lZoomNumerator,		//@parm Zoom numerator
	LONG lZoomDenominator,		//@parm Zoom denominator
	LONG yPixelsPerInch,		//@parm Pixels per inch for hdc
	LONG *pxAveWidth,			//@parm Optional ave width of character
	LONG *pxOverhang,			//@parm Optional overhang
	LONG *pxUnderhang)			//@parm Optional underhang
{
	// Convert the text
	CTxtEdit *ped = (CTxtEdit *) pts;

	// Get the CCcs that has all the information we need
	CCcs *pccs = fc().GetCcs(hdc, ped->GetCharFormat(-1), lZoomNumerator,
		lZoomDenominator, yPixelsPerInch);

	if (pxAveWidth != NULL)
	{
		// Return the information
		*pxAveWidth = pccs->_xAveCharWidth;
	}

	if (pxOverhang != NULL)
	{
		// Return the overhang
		*pxOverhang = pccs->_xOverhang;
	}

	if (pxUnderhang != NULL)
	{
		// Return the overhang
		*pxUnderhang = pccs->_xUnderhang;
	}

	LONG yHeight = pccs->_yHeight;

	// Release the CCcs
	pccs->Release();

	return yHeight;
}

/* 
 *	CTxtEdit::TxScrollWindowEx (dx, dy, lprcScroll, lprcClip, hrgnUpdate,
 *									lprcUpdate, fuScroll)
 *	@mfunc
 *		Request Text Host to scroll the content of the specified client area
 *
 *	@comm
 *		This method is only valid when the control is in-place active;
 *		calls while inactive may fail.
 */
void CTxtEdit::TxScrollWindowEx (
	INT		dx, 			//@parm	Amount of horizontal scrolling
	INT		dy, 			//@parm	Amount of vertical scrolling
	LPCRECT lprcScroll, 	//@parm	Scroll rectangle
	LPCRECT lprcClip,		//@parm	Clip rectangle
	HRGN	hrgnUpdate, 	//@parm	Handle of update region
	LPRECT	lprcUpdate,		//@parm	Update rectangle
	UINT	fuScroll )		//@parm	Scrolling flags
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEEXTERN, "CTxtEdit::TxScrollWindowEx");

	if(_fInPlaceActive)
	{
#ifndef MACPORT
		CMagellanBMPStateWrap bmpOff(*this, NULL);
#endif

		_phost->TxScrollWindowEx(dx, dy, lprcScroll, lprcClip,
						hrgnUpdate, lprcUpdate, fuScroll);

		// now go tell all the objects that they may need to update their position
		// rects if scrolling occurred.

		if( _pobjmgr )
		{
			RECT rcClient;

			if( !lprcScroll )
			{
				TxGetClientRect(&rcClient);
				lprcScroll = &rcClient;
			}
				
			_pobjmgr->ScrollObjects(dx, dy, lprcScroll);
		}
	}
}

// FUTURE: remove the MAC conditionalization.			
#ifndef _MAC
/*
 *	CTxtEdit::HandleMouseWheel
 *
 *	@mfunc	Handles scrolling as a result of rotating a mouse roller wheel.
 *
 *	@rdesc	LRESULT
 */
LRESULT	CTxtEdit::HandleMouseWheel(WPARAM wparam, LPARAM lparam)
{ 	
	// this bit of global state is OK
	static LONG gcWheelDelta = 0;
	short zdelta = (short)HIWORD(wparam);

	// we don't handle zoom or data zoom
	if( (wparam & (MK_SHIFT | MK_CONTROL)) )
	{
		return 0;
	}

	gcWheelDelta += zdelta;

	if( abs(gcWheelDelta) >= WHEEL_DELTA )
	{
		LONG cLineScroll = sysparam.GetRollerLineScrollCount();

		if( cLineScroll != -1 )
		{
			cLineScroll *= ((abs(gcWheelDelta))/WHEEL_DELTA);
		}
		gcWheelDelta %= WHEEL_DELTA;

		// -1 means scroll by pages; so simply call page up/down. 
		if( cLineScroll == -1 )
		{
			if( _pdp )
			{
				if( zdelta < 0 )
				{
					_pdp->VScroll(SB_PAGEDOWN, 0);
				}
				else
				{
					_pdp->VScroll(SB_PAGEUP, 0);
				}
			}
			return TRUE;
		}

		mouse.MagellanRollScroll( _pdp, zdelta, cLineScroll, 
				SMOOTH_ROLL_NUM, SMOOTH_ROLL_DENOM, TRUE);
		return TRUE;
	}

	return 0;
}
#endif //_MAC
