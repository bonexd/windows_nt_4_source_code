/****************************************************************************
*
*  HCCOM.CPP
*
*  Copyright (C) Microsoft Corporation 1993-1994
*  All Rights reserved.
*
*  Code common to HCW.EXE and HCRTF.EXE
*
*****************************************************************************/

#include "stdafx.h"

#ifndef HCCOM_H
#include "..\common\hccom.h"
#endif

#ifndef _MAX_FNAME
#include <stdlib.h>
#include <stdio.h>
#endif

#ifndef _COMMON_H
#include "..\common\common.h"
#endif

#include "..\common\resource.h"
#include "..\common\cstr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

////////////////// GLOBALS ///////////////////////////

HWND hwndParent = NULL;		// HCW.EXE's window handle when this module is hcrtf
HGLOBAL hmemSz = NULL; 		// HCRTF's shared memory handle
HINSTANCE hinstApp = NULL;

char txtHCWClass[] = "hcw_class";
char txtTmpName[]  = "~hc";
const char txtZeroLength[] = "";
const char txtSharedMem[] = "hcshare";

// Must be in same order as OPT enumeration in hccom.h

const PSTR ppszOptions[] = {
	"BMROOT",
	"BUILD",
	"COMPRESS",
	"ERRORLOG",
	"FORCEFONT",
	"ICON",
	"CONTENTS",
	"LANGUAGE",
	"MAPFONTSIZE",
	"MULTIKEY",
	"REPORT",
	"ROOT",
	"TITLE",
	"OLDKEYPHRASE",
	"WARNING",
	"COPYRIGHT",
	"OPTCDROM",
	"CITATION",

	// New for version 4.0

	"VERSION",
	"NOTES",
	"CNT",
	"HLP",
	"HCW",
	"LCID",
	"DBCS",
	"TMPDIR",
	"REPLACE",
	"CHARSET",
	"FTS",
	"DEFFONT",
	"PREFIX",
	"REVISIONS",

	// New for 4.01 (except OPT_INDEX)

	"IGNORE",

	"INDEX_SEPARATORS",
};

const MACRO_PAIR macropair[] = {
	{ "AA(",   "AddAccelerator(", },
	{ "AI(",   "AppendItem(", },
	{ "AL(",   "ALink(", },
	{ "AN(",   "Annotate(", },
	{ "CB(",   "CreateButton(", },
	{ "CBB(",  "ChangeButtonBinding(", },
	{ "CE(",   "ChangeEnable(", },
	{ "CI(",   "CheckItem(", },
	{ "CIB(",  "ChangeItemBinding(", },
	{ "CS(",   "CloseSecondarys(", },
	{ "CT(",   "CopyTopic(", },
	{ "CW(",   "CloseWindow(", },
	{ "DB(",   "DisableButton(", },
	{ "DEB(",  "DestroyButton(", },
	{ "DI(",   "DisableItem(", },
	{ "EB(",   "EnableButton(", },
	{ "EF(",   "ExecFile(", },
	{ "EI(",   "EnableItem(", },
	{ "EP(",   "ExecProgram(", },
	{ "FD(",   "Finder(", },
	{ "FE(",   "FileExist(", },
	{ "FH(",   "FlushMessageQueue(", },
	{ "FO(",   "FileOpen(", },
	{ "IB(",   "IsBook(", },
	{ "IE(",   "IfThenElse(", },
	{ "IF(",   "IfElse(", },
	{ "JC(",   "JumpContext(", },
	{ "JI(",   "JumpId(", },
	{ "JK(",   "JumpKeyword(", },
	{ "JW(",   "JumpWindow(", },
	{ "KL(",   "KLink(", },
	{ "MU(",   "Menu(", },
	{ "NS(",   "NoShow(", },
	{ "PC(",   "PopupContext(", },
	{ "PI(",   "PopupId(", },
	{ "RA(",   "RemoveAccelerator(", },
	{ "RR(",   "RegisterRoutine(", },
	{ "SE(",   "ShellExecute(", },
	{ "SF(",   "ShowFolder(", },
	{ "SH(",   "ShortCut(", },
	{ "SPC(",  "SetPopupColor(", },
	{ "SW(",   "ShowInWindow(", },
	{ "TC(",   "TCard(", },
	{ "UI(",   "UncheckItem(", },

	{ NULL, NULL, },
};

const int MAX_OPT = ELEMENTS(ppszOptions);

////////////////// ROUTINES //////////////////////////

#include <dos.h>

void STDCALL AssertErrorReport(PCSTR pszExpression, UINT line,
	PCSTR pszFile)
{
	char	 szBuf[512], szErrorFile[30];
	char	 szExpression[256];
	HFILE	 hf;
	OFSTRUCT of;
	static BOOL fAsserted = FALSE;

	if (fAsserted)
		return;    // we already asserted
	else
		fAsserted = TRUE;

	/*
	 * Often the expression will have been obtained via GetStringResource,
	 * so we make a copy of it here to save the information.
	 */

	lstrcpyn(szExpression, pszExpression, sizeof(szExpression));

	// REVIEW: ultimately, we should use the Mail API to do this.

	char szName[_MAX_FNAME];

#ifdef INTERNAL
	if (!GetVolumeInformation("c:\\", szName, sizeof(szName),
			NULL, NULL, NULL, NULL, 0)) {
		strcpy(szName, "hc");
	}
	else {
		szName[8] = '\0';
		CharLower(szName);
		strcat(szName, ".err");
		strcpy(szErrorFile, "\\\\pike\\bugs\\");
		strcat(szErrorFile, szName);
	}

	of.cBytes = sizeof(OFSTRUCT);
	hf = OpenFile(szErrorFile, &of, OF_CREATE | OF_WRITE);
	if (hf == HFILE_ERROR) {

		// couldn't find \\pike, so copy it to their C drive.

		strcpy(szErrorFile, "c:\\");
		strcat(szErrorFile, szName);
		strcat(szErrorFile, ".err");
		hf = OpenFile(szErrorFile, &of, OF_CREATE | OF_WRITE);
	}

#else

	if (!GetVolumeInformation("c:\\", szName, sizeof(szName),
			NULL, NULL, NULL, NULL, 0)) {
		strcpy(szErrorFile, "hc");
	}
	else {
		strcpy(szErrorFile, "c:\\");
		_strlwr(szName);
		strcat(szErrorFile, szName);
		szErrorFile[10] = '\0';
		strcat(szErrorFile, ".err");
	}

	of.cBytes = sizeof(OFSTRUCT);
	hf = OpenFile(szErrorFile, &of, OF_CREATE | OF_WRITE);

#endif

	if (hf >= 0) {
		strcpy(szBuf, GetStringResource(IDS_VERSION));
		wsprintf(szBuf + strlen(szBuf),
			GetStringResource(IDS_ASSERTION_FAILURE),
			pszFile, line, szExpression);
		_lwrite(hf, szBuf, strlen(szBuf));

		wsprintf(szBuf, GetStringResource(IDS_ASSRT_COPY_MSG),
			szErrorFile);
		szMsgBox(szBuf);
		_lclose(hf);
	}
	else {
		MsgBox(IDS_INTERNAL_ERROR);
	}

	if (hwndParent) { // Usually non-null for the help compiler
		throw 0;
	}

#ifdef _DEBUG
	int answer = MessageBox(NULL, szExpression, "Retry to call DebugBreak()",
		MB_ABORTRETRYIGNORE);

	if (answer == IDRETRY) {
		DebugBreak();
		return;
	}
	else if (answer == IDIGNORE)
		return;
#endif
	FatalAppExit(0, GetStringResource(IDS_ASSERTION_ERROR));
	return;
}

static char szStringBuf[256];

PCSTR STDCALL GetStringResource(UINT idString)
{
		if (LoadString(hinstApp, idString, szStringBuf,
				sizeof(szStringBuf)) == 0) {
#ifdef _DEBUG
			DebugBreak();
			wsprintf(szStringBuf, "Cannot find string resource id %d.",
				idString);
			MessageBox(NULL, szStringBuf, "", MB_OK);
#endif
			szStringBuf[0] = '\0';
		}
		return (const PSTR) szStringBuf;
}

void STDCALL GetStringResource(UINT idString, PSTR pszDst)
{
		if (LoadString(hinstApp, idString, pszDst, 256) == 0) {
#ifdef _DEBUG
			DebugBreak();
			wsprintf(pszDst, "Cannot find string resource id %d.", idString);
			MessageBox(NULL, pszDst, "", MB_OK);
#endif
			*pszDst = '\0';
		}
}

/***************************************************************************

	FUNCTION:	ChangeExtension

	PURPOSE:	This version of change extension assumes the extension
				is contained in a resource id

	PARAMETERS:
		pszDest
		idResource

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		16-Oct-1993 [ralphw]

***************************************************************************/

void STDCALL ChangeExtension(PSTR pszDest, UINT idResource)
{
	PSTR psz;
	static BOOL fDBCSSystem = IsDbcsSystem();

	PSTR pszExt = (PSTR) GetStringResource(idResource);

	// If NULL specified, simply remove any existing extension

	if (pszExt == NULL || !*pszExt) {
		if ((psz = StrRChr(pszDest, '.', fDBCSSystem)) != NULL)
			*psz = '\0';
		return;
	}

	if ((psz = StrRChr(pszDest, '.', fDBCSSystem)) == NULL)
		psz = pszDest + strlen(pszDest);  // filename didn't have an extension
	if (*pszExt != '.')
		*psz++ = '.';
	strcpy(psz, pszExt);
}

/***************************************************************************

	FUNCTION:	DeleteTmpFiles

	PURPOSE:	Delete all temporary files created by hcrtf

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:
		This will also delete old temporary files from a crashed
		hcrtf. By calling it from both HCW and HCRTF we can be assured that
		we have cleaned up after ourselves.

	MODIFICATION DATES:
		26-Aug-1993 [ralphw]
		01-Jun-1994 [ralphw] Temporary files are now in TMP

***************************************************************************/

#include <dos.h>

void STDCALL DeleteTmpFiles(void)
{
	char szTmpName[_MAX_PATH];
	
	strcpy(szTmpName, GetTmpDirectory());
	CStr cszPath(szTmpName);
	strcat(szTmpName, txtTmpName);
	strcat(szTmpName, "*.tmp");

	WIN32_FIND_DATA fileinfo;
	HANDLE hfind = FindFirstFile(szTmpName, &fileinfo);

	if (hfind != INVALID_HANDLE_VALUE) {
		do {
			strcpy(szTmpName, cszPath);
			strcat(szTmpName, fileinfo.cFileName);
			remove(szTmpName);
		} while (FindNextFile(hfind, &fileinfo));
		FindClose(hfind);
	}
}

/*
 * Yes/No/Neither - recognizes various binary options
 *
 * We don't put this into the resource, because the text is supported for
 * backwards compatibility only. HCW-maintained projects use 0 and 1 for
 * FALSE and TRUE respectively.
 */

static const PSTR ppszYesNo[] = {
	"no",	  "yes",
	"false",  "true",
	"off",	  "on",
	"0",	  "1",
};

static const int MAX_YESNO = ELEMENTS(ppszYesNo);

int STDCALL YesNo(PCSTR psz)
{
	int i;

	for (i = 0; i < MAX_YESNO; i++) {
		if (!_stricmp(psz, ppszYesNo[i]))
			return i % 2 ? IDYES : IDNO;
	}
	return IDCANCEL;
}

/***************************************************************************

	FUNCTION:	GetArg

	PURPOSE:

	PARAMETERS:
		pszDest
		pszSrc

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		16-Aug-1993 [ralphw]

***************************************************************************/

PSTR STDCALL GetArg(PSTR pszDest, PCSTR pszSrc)
{
	static BOOL fDBCSSystem = IsDbcsSystem();

	if (!pszSrc) {
		*pszDest = '\0';
		return NULL;
	}
	if (*pszSrc == CH_QUOTE) {
		pszSrc++;
		while (*pszSrc && *pszSrc != CH_QUOTE)
			*pszDest++ = *pszSrc++;
	}
	else if (*pszSrc == CH_START_QUOTE) {
		pszSrc++;
		while (*pszSrc && *pszSrc != CH_END_QUOTE)
			*pszDest++ = *pszSrc++;
	}
	else {
		while (*pszSrc && !isspace(*pszSrc))
			*pszDest++ = *pszSrc++;
	}
	*pszDest = '\0';
	return FirstNonSpace(pszSrc, fDBCSSystem);
}

void STDCALL AddTrailingBackslash(PSTR npszStr)
{
	int sPos;

	if (npszStr != NULL && *npszStr != '\0') {

		sPos = strlen(npszStr) - 1;

		if (npszStr[sPos] != '\\') {
			npszStr[sPos + 1] = '\\';
			npszStr[sPos + 2] = '\0';
		}
	}
}

#include <direct.h>

void STDCALL ChangeDirectory(PCSTR pszFile)
{
	char szPath[_MAX_PATH];
	strcpy(szPath, pszFile);
	PSTR psz = StrRChr(szPath, CH_BACKSLASH, IsDbcsSystem());
	if (!psz)
		return;
	else
		*psz = '\0';

	if (_chdir(szPath) != 0)
		return;

	if (szPath[1] == ':')
		_chdrive(tolower(szPath[0]) - ('a' - 1));
}

/***************************************************************************

	FUNCTION:	FGetNum

	PURPOSE:	Get a signed or unsigned 32-bit number

	PARAMETERS:
		pszIn	Points to the number
		ppszOut On return, Points to the first non-space after the number
		pv		Where to put the value
		fSigned TRUE if you want a signed number

	RETURNS:	TRUE if the first character is a number

	COMMENTS:

	MODIFICATION DATES:
		26-Jun-1994 [ralphw]
			ppszOut can be NULL
			*pv is set to zero on failure

***************************************************************************/

BOOL STDCALL FGetNum(PCSTR pszIn, PSTR *ppszOut, void* pv, BOOL fSigned)
{
	PSTR pszTmp;
	PSTR* ppszEnd = ppszOut ? ppszOut : &pszTmp;
	static BOOL fDBCSSystem = IsDbcsSystem();

	if (!(isdigit(*pszIn) || (*pszIn == '-' && fSigned))) {
		*(int *)pv = 0;
		return FALSE;
	}

	if (fSigned) {
		int l = strtol(pszIn, ppszEnd, 0);
		*(int *)pv = l;
	}
	else {
		DWORD ul = strtoul(pszIn, ppszEnd, 0);
		*(DWORD *) pv = (UINT) ul;
	}

	if (ppszOut) {
		// In case caller specifed a long value

		if ((**ppszEnd == 'l' || **ppszEnd == 'L'))
			++*ppszEnd;

		*ppszEnd = FirstNonSpace(*ppszOut, fDBCSSystem);
	}
	return TRUE;
}

/***************************************************************************

	FUNCTION:	ReplaceStrings

	PURPOSE:	Replace one string with another in a string

	PARAMETERS:
		pszCur	-- pointer to the buffer
		pszOrg	-- original string to be replaced
		pszNew	-- string to replace with

	RETURNS:

	COMMENTS:
		This will move the buffer to make room for the new string

	MODIFICATION DATES:
		08-Mar-1995 [ralphw]

***************************************************************************/

void STDCALL ReplaceStrings(PSTR pszCur, PCSTR pszOrg, PCSTR pszNew)
{
	ASSERT(nstrisubcmp(pszCur, pszOrg));
	int cbNew = strlen(pszNew);
	int cbOrg = strlen(pszOrg);

	if (cbNew < cbOrg) {
		strcpy(pszCur, pszNew);
		strcpy(pszCur + cbNew, pszCur + cbOrg);
	}
	else {
		MoveMemory(pszCur + strlen(pszNew) - strlen(pszOrg), pszCur,
			strlen(pszCur) + 1);
		while (*pszNew)
			*pszCur++ = *pszNew++;
	}
}

const char txtAddAcclerator[] = "AddAccelerator(";

const MACRO_PAIR vkpair[] = {
	{ "0x08",	"VK_BACK", },
	{ "0x09",	"VK_TAB", },
	{ "0x0D",	"VK_RETURN", },
	{ "0x1B",	"VK_ESCAPE", },
	{ "0x20",	"VK_SPACE", },
	{ "0x21",	"VK_PRIOR", },
	{ "0x22",	"VK_NEXT", },
	{ "0x23",	"VK_END", },
	{ "0x24",	"VK_HOME", },
	{ "0x25",	"VK_LEFT", },
	{ "0x26",	"VK_UP", },
	{ "0x27",	"VK_RIGHT", },
	{ "0x28",	"VK_DOWN", },
	{ "0x2D",	"VK_INSERT", },
	{ "0x2E",	"VK_DELETE", },
	{ "0x2F",	"VK_HELP", },

	{ "0x30",	"\'0\'", },
	{ "0x31",	"\'1\'", },
	{ "0x32",	"\'2\'", },
	{ "0x33",	"\'3\'", },
	{ "0x34",	"\'4\'", },
	{ "0x35",	"\'5\'", },
	{ "0x36",	"\'6\'", },
	{ "0x37",	"\'7\'", },
	{ "0x38",	"\'8\'", },
	{ "0x39",	"\'9\'", },

	{ "0x41",	"\'A\'", },
	{ "0x42",	"\'B\'", },
	{ "0x43",	"\'C\'", },
	{ "0x44",	"\'D\'", },
	{ "0x45",	"\'E\'", },
	{ "0x46",	"\'F\'", },
	{ "0x47",	"\'G\'", },
	{ "0x48",	"\'H\'", },
	{ "0x49",	"\'I\'", },
	{ "0x4A",	"\'J\'", },
	{ "0x4B",	"\'K\'", },
	{ "0x4C",	"\'L\'", },
	{ "0x4D",	"\'M\'", },
	{ "0x4E",	"\'N\'", },
	{ "0x4F",	"\'O\'", },
	{ "0x50",	"\'P\'", },
	{ "0x51",	"\'Q\'", },
	{ "0x52",	"\'R\'", },
	{ "0x53",	"\'S\'", },
	{ "0x54",	"\'T\'", },
	{ "0x55",	"\'U\'", },
	{ "0x56",	"\'V\'", },
	{ "0x57",	"\'W\'", },
	{ "0x58",	"\'X\'", },
	{ "0x59",	"\'Y\'", },
	{ "0x5A",	"\'Z\'", },

	{ "0x60",	"VK_NUMPAD0", },
	{ "0x61",	"VK_NUMPAD1", },
	{ "0x62",	"VK_NUMPAD2", },
	{ "0x63",	"VK_NUMPAD3", },
	{ "0x64",	"VK_NUMPAD4", },
	{ "0x65",	"VK_NUMPAD5", },
	{ "0x66",	"VK_NUMPAD6", },
	{ "0x67",	"VK_NUMPAD7", },
	{ "0x68",	"VK_NUMPAD8", },
	{ "0x69",	"VK_NUMPAD9", },
	{ "0x6A",	"VK_MULTIPLY", },
	{ "0x6B",	"VK_ADD", },
	{ "0x6C",	"VK_SEPARATOR", },
	{ "0x6D",	"VK_SUBTRACT", },
	{ "0x6E",	"VK_DECIMAL", },
	{ "0x6F",	"VK_DIVIDE", },

	{ "0x70",	"VK_F1", },
	{ "0x71",	"VK_F2", },
	{ "0x72",	"VK_F3", },
	{ "0x73",	"VK_F4", },
	{ "0x74",	"VK_F5", },
	{ "0x75",	"VK_F6", },
	{ "0x76",	"VK_F7", },
	{ "0x77",	"VK_F8", },
	{ "0x78",	"VK_F9", },
	{ "0x79",	"VK_F10", },
	{ "0x7A",	"VK_F11", },
	{ "0x7B",	"VK_F12", },
	{ "0x7C",	"VK_F13", },
	{ "0x7D",	"VK_F14", },
	{ "0x7E",	"VK_F15", },
	{ "0x7F",	"VK_F16", },
	{ "0x80",	"VK_F17", },
	{ "0x81",	"VK_F18", },
	{ "0x82",	"VK_F19", },
	{ "0x83",	"VK_F20", },
	{ "0x84",	"VK_F21", },
	{ "0x85",	"VK_F22", },
	{ "0x86",	"VK_F23", },
	{ "0x87",	"VK_F24", },

	{ NULL, NULL, },
};

const MACRO_PAIR vkshift[] = {
	{ "7", "ALT+CTRL+SHIFT", },
	{ "6", "ALT+CTRL",	   },
	{ "5", "ALT+SHIFT",	   },
	{ "4", "ALT", 		   },
	{ "3", "CTRL+SHIFT",	   },
	{ "2", "CTRL",		   },
	{ "1", "SHIFT",		   },
	{ "0", "NONE",		   },

	{ NULL, NULL, },
};

void STDCALL TweakAddAccelerator(PSTR psz)
{
	if (nstrisubcmp(psz, txtAddAcclerator)) {
		psz = FirstNonSpace(psz + strlen(txtAddAcclerator));
		int i;
		for (i = 0; vkpair[i].pszShort; i++) {
			if (nstrisubcmp(psz, vkpair[i].pszShort)) {
				ReplaceStrings(psz, vkpair[i].pszShort, vkpair[i].pszExpanded);
				break;
			}
		}

		psz = strchr(psz, ',');
		if (psz)
			psz = FirstNonSpace(psz + 1);

		for (i = 0; vkshift[i].pszShort; i++) {
			if (nstrisubcmp(psz, vkshift[i].pszShort)) {
				ReplaceStrings(psz, vkshift[i].pszShort, vkshift[i].pszExpanded);
				break;
			}
		}
	}
}

/***************************************************************************
 *
 -	Name:		 FValidContextSz
 -
 *	Purpose:
 *	  This function determines whether the given string may be
 *	used as a context string.
 *
 *	Arguments:
 *	  SZ:  String to validate.
 *
 *	Returns:
 *	  TRUE if the string is a valid context string, FALSE otherwise.
 *
 ***************************************************************************/

BOOL STDCALL IsValidContextSz(PCSTR pszContext)
{
	/*
	 * To avoid confusion with macro strings, context strings may not begin
	 * with an exclamation point.
	 */

	if (*pszContext == CH_MACRO || *pszContext == '\0')
		return FALSE;

	// Version 4.x help files have almost no limitations on context strings

	if (strpbrk(pszContext, "#=>@%;"))
		return FALSE;
	else
		return TRUE;
}
