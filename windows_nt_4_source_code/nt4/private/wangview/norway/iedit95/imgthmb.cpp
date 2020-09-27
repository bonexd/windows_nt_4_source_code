// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


#include "stdafx.h"
#include "imgthmb.h"

/////////////////////////////////////////////////////////////////////////////
// CImgThumbnail

IMPLEMENT_DYNCREATE(CImgThumbnail, CWnd)

/////////////////////////////////////////////////////////////////////////////
// CImgThumbnail properties

long CImgThumbnail::GetThumbCount()
{
	long result;
	GetProperty(0x1, VT_I4, (void*)&result);
	return result;
}

void CImgThumbnail::SetThumbCount(long propVal)
{
	SetProperty(0x1, VT_I4, propVal);
}

long CImgThumbnail::GetThumbWidth()
{
	long result;
	GetProperty(0x2, VT_I4, (void*)&result);
	return result;
}

void CImgThumbnail::SetThumbWidth(long propVal)
{
	SetProperty(0x2, VT_I4, propVal);
}

long CImgThumbnail::GetThumbHeight()
{
	long result;
	GetProperty(0x3, VT_I4, (void*)&result);
	return result;
}

void CImgThumbnail::SetThumbHeight(long propVal)
{
	SetProperty(0x3, VT_I4, propVal);
}

long CImgThumbnail::GetScrollDirection()
{
	long result;
	GetProperty(0x4, VT_I4, (void*)&result);
	return result;
}

void CImgThumbnail::SetScrollDirection(long propVal)
{
	SetProperty(0x4, VT_I4, propVal);
}

long CImgThumbnail::GetThumbCaptionStyle()
{
	long result;
	GetProperty(0x5, VT_I4, (void*)&result);
	return result;
}

void CImgThumbnail::SetThumbCaptionStyle(long propVal)
{
	SetProperty(0x5, VT_I4, propVal);
}

unsigned long CImgThumbnail::GetThumbCaptionColor()
{
	unsigned long result;
	GetProperty(0x6, VT_I4, (void*)&result);
	return result;
}

void CImgThumbnail::SetThumbCaptionColor(unsigned long propVal)
{
	SetProperty(0x6, VT_I4, propVal);
}

LPDISPATCH CImgThumbnail::GetThumbCaptionFont()
{
	LPDISPATCH result;
	GetProperty(0x7, VT_DISPATCH, (void*)&result);
	return result;
}

void CImgThumbnail::SetThumbCaptionFont(LPDISPATCH propVal)
{
	SetProperty(0x7, VT_DISPATCH, propVal);
}

BOOL CImgThumbnail::GetHighlightSelectedThumbs()
{
	BOOL result;
	GetProperty(0x8, VT_BOOL, (void*)&result);
	return result;
}

void CImgThumbnail::SetHighlightSelectedThumbs(BOOL propVal)
{
	SetProperty(0x8, VT_BOOL, propVal);
}

long CImgThumbnail::GetSelectedThumbCount()
{
	long result;
	GetProperty(0x9, VT_I4, (void*)&result);
	return result;
}

void CImgThumbnail::SetSelectedThumbCount(long propVal)
{
	SetProperty(0x9, VT_I4, propVal);
}

long CImgThumbnail::GetFirstSelectedThumb()
{
	long result;
	GetProperty(0xa, VT_I4, (void*)&result);
	return result;
}

void CImgThumbnail::SetFirstSelectedThumb(long propVal)
{
	SetProperty(0xa, VT_I4, propVal);
}

long CImgThumbnail::GetLastSelectedThumb()
{
	long result;
	GetProperty(0xb, VT_I4, (void*)&result);
	return result;
}

void CImgThumbnail::SetLastSelectedThumb(long propVal)
{
	SetProperty(0xb, VT_I4, propVal);
}

CString CImgThumbnail::GetThumbCaption()
{
	CString result;
	GetProperty(0xc, VT_BSTR, (void*)&result);
	return result;
}

void CImgThumbnail::SetThumbCaption(LPCTSTR propVal)
{
	SetProperty(0xc, VT_BSTR, propVal);
}

unsigned long CImgThumbnail::GetHighlightColor()
{
	unsigned long result;
	GetProperty(0xd, VT_I4, (void*)&result);
	return result;
}

void CImgThumbnail::SetHighlightColor(unsigned long propVal)
{
	SetProperty(0xd, VT_I4, propVal);
}

unsigned long CImgThumbnail::GetThumbBackColor()
{
	unsigned long result;
	GetProperty(0xe, VT_I4, (void*)&result);
	return result;
}

void CImgThumbnail::SetThumbBackColor(unsigned long propVal)
{
	SetProperty(0xe, VT_I4, propVal);
}

long CImgThumbnail::GetStatusCode()
{
	long result;
	GetProperty(0xf, VT_I4, (void*)&result);
	return result;
}

void CImgThumbnail::SetStatusCode(long propVal)
{
	SetProperty(0xf, VT_I4, propVal);
}

CString CImgThumbnail::GetImage()
{
	CString result;
	GetProperty(0x10, VT_BSTR, (void*)&result);
	return result;
}

void CImgThumbnail::SetImage(LPCTSTR propVal)
{
	SetProperty(0x10, VT_BSTR, propVal);
}

long CImgThumbnail::GetMousePointer()
{
	long result;
	GetProperty(0x11, VT_I4, (void*)&result);
	return result;
}

void CImgThumbnail::SetMousePointer(long propVal)
{
	SetProperty(0x11, VT_I4, propVal);
}

LPDISPATCH CImgThumbnail::GetMouseIcon()
{
	LPDISPATCH result;
	GetProperty(0x12, VT_DISPATCH, (void*)&result);
	return result;
}

void CImgThumbnail::SetMouseIcon(LPDISPATCH propVal)
{
	SetProperty(0x12, VT_DISPATCH, propVal);
}

OLE_COLOR CImgThumbnail::GetBackColor()
{
	OLE_COLOR result;
	GetProperty(DISPID_BACKCOLOR, VT_I4, (void*)&result);
	return result;
}

void CImgThumbnail::SetBackColor(OLE_COLOR propVal)
{
	SetProperty(DISPID_BACKCOLOR, VT_I4, propVal);
}

short CImgThumbnail::GetBorderStyle()
{
	short result;
	GetProperty(DISPID_BORDERSTYLE, VT_I2, (void*)&result);
	return result;
}

void CImgThumbnail::SetBorderStyle(short propVal)
{
	SetProperty(DISPID_BORDERSTYLE, VT_I2, propVal);
}

BOOL CImgThumbnail::GetEnabled()
{
	BOOL result;
	GetProperty(DISPID_ENABLED, VT_BOOL, (void*)&result);
	return result;
}

void CImgThumbnail::SetEnabled(BOOL propVal)
{
	SetProperty(DISPID_ENABLED, VT_BOOL, propVal);
}

OLE_HANDLE CImgThumbnail::GetHWnd()
{
	OLE_HANDLE result;
	GetProperty(DISPID_HWND, VT_I4, (void*)&result);
	return result;
}

void CImgThumbnail::SetHWnd(OLE_HANDLE propVal)
{
	SetProperty(DISPID_HWND, VT_I4, propVal);
}

long CImgThumbnail::GetFirstDisplayedThumb()
{
	long result;
	GetProperty(0x13, VT_I4, (void*)&result);
	return result;
}

void CImgThumbnail::SetFirstDisplayedThumb(long propVal)
{
	SetProperty(0x13, VT_I4, propVal);
}

long CImgThumbnail::GetLastDisplayedThumb()
{
	long result;
	GetProperty(0x14, VT_I4, (void*)&result);
	return result;
}

void CImgThumbnail::SetLastDisplayedThumb(long propVal)
{
	SetProperty(0x14, VT_I4, propVal);
}

/////////////////////////////////////////////////////////////////////////////
// CImgThumbnail operations

void CImgThumbnail::SelectAllThumbs()
{
	InvokeHelper(0x65, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void CImgThumbnail::DeselectAllThumbs()
{
	InvokeHelper(0x66, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

long CImgThumbnail::GetMinimumSize(long ThumbCount, BOOL ScrollBar)
{
	long result;
	static BYTE parms[] =
		VTS_I4 VTS_BOOL;
	InvokeHelper(0x67, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		ThumbCount, ScrollBar);
	return result;
}

long CImgThumbnail::GetMaximumSize(long ThumbCount, BOOL ScrollBar)
{
	long result;
	static BYTE parms[] =
		VTS_I4 VTS_BOOL;
	InvokeHelper(0x68, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		ThumbCount, ScrollBar);
	return result;
}

void CImgThumbnail::ClearThumbs(const VARIANT& PageNumber)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x69, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 &PageNumber);
}

void CImgThumbnail::InsertThumbs(const VARIANT& InsertBeforeThumb, const VARIANT& InsertCount)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x6a, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 &InsertBeforeThumb, &InsertCount);
}

void CImgThumbnail::DeleteThumbs(long DeleteAt, const VARIANT& DeleteCount)
{
	static BYTE parms[] =
		VTS_I4 VTS_VARIANT;
	InvokeHelper(0x6b, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 DeleteAt, &DeleteCount);
}

void CImgThumbnail::DisplayThumbs(const VARIANT& ThumbNumber, const VARIANT& Option)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x6c, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 &ThumbNumber, &Option);
}

void CImgThumbnail::GenerateThumb(short Option, const VARIANT& PageNumber)
{
	static BYTE parms[] =
		VTS_I2 VTS_VARIANT;
	InvokeHelper(0x6d, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Option, &PageNumber);
}

BOOL CImgThumbnail::ScrollThumbs(short Direction, short Amount)
{
	BOOL result;
	static BYTE parms[] =
		VTS_I2 VTS_I2;
	InvokeHelper(0x6e, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms,
		Direction, Amount);
	return result;
}

BOOL CImgThumbnail::UISetThumbSize(const VARIANT& Image, const VARIANT& Page)
{
	BOOL result;
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x6f, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms,
		&Image, &Page);
	return result;
}

long CImgThumbnail::GetScrollDirectionSize(long ScrollDirectionThumbCount, long NonScrollDirectionThumbCount, long NonScrollDirectionSize, BOOL ScrollBar)
{
	long result;
	static BYTE parms[] =
		VTS_I4 VTS_I4 VTS_I4 VTS_BOOL;
	InvokeHelper(0x70, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		ScrollDirectionThumbCount, NonScrollDirectionThumbCount, NonScrollDirectionSize, ScrollBar);
	return result;
}

void CImgThumbnail::Refresh()
{
	InvokeHelper(DISPID_REFRESH, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

long CImgThumbnail::GetThumbPositionX(long ThumbNumber)
{
	long result;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x71, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		ThumbNumber);
	return result;
}

long CImgThumbnail::GetThumbPositionY(long ThumbNumber)
{
	long result;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x72, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		ThumbNumber);
	return result;
}

CString CImgThumbnail::GetVersion()
{
	CString result;
	InvokeHelper(0x73, DISPATCH_METHOD, VT_BSTR, (void*)&result, NULL);
	return result;
}

BOOL CImgThumbnail::GetThumbSelected(long PageNumber)
{
	BOOL result;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xc8, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, parms,
		PageNumber);
	return result;
}

void CImgThumbnail::SetThumbSelected(long PageNumber, BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_I4 VTS_BOOL;
	InvokeHelper(0xc8, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 PageNumber, bNewValue);
}

void CImgThumbnail::AboutBox()
{
	InvokeHelper(0xfffffdd8, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}
