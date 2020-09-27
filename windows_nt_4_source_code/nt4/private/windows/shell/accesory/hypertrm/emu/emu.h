/*	File: D:\WACKER\emu\emu.h (Created: 08-Dec-1993)
 *
 *	Copyright 1994 by Hilgraeve Inc. -- Monroe, MI
 *	All rights reserved
 *
 *	$Revision: 1.57 $
 *	$Date: 1995/04/13 16:47:31 $
 */

/* Emulator ID's */

// Don't ever change emulator id numbers.  They are stored in session files
// and as such are cast in stone. - mrw,4/13/95
//
#define EMU_AUTO						100
#define EMU_ANSI						101
#define EMU_MINI						102
#define EMU_VIEW						109
#define EMU_TTY 						110
#define EMU_VT100						111
#define EMU_VT52						115
#define EMU_VT100J						116
#define EMU_ANSIW						117

#define NBR_EMULATORS					9

/* Emulator constants  */

// Note: if you change MAX_EMUROWS or MAX_EMUCOLS also change
//		 TERM_ROWS and TERM_COLS in term.hh to match - mrw

#define MAX_EMUROWS 	32		// Largest vertical size for emulator.
#define MAX_EMUCOLS 	132		// Largest horizontal size for emulator.
#define MIN_EMUROWS 	2		// Smallest allowable value for rows,
#define MIN_EMUCOLS 	2		// and columns.

#define EMU_DEFAULT_COLS			80
#define EMU_DEFAULT_ROWS			24

#define EMU_OK						0
#define TRM_NUMSTRIPCHARS			3
#define EMU_MAX_NAMELEN 			15
#define EMU_MAX_AUTODETECT_ATTEMPTS 10

// JCM should remove the two defines below.
//
#define EMU_DEFAULT_MAXCOL			79
#define EMU_DEFAULT_MAXROW			23

#define EMU_KEYS_ACCEL		0
#define EMU_KEYS_TERM		1
#define EMU_KEYS_SCAN		2

#define EMU_CURSOR_BLOCK	1
#define EMU_CURSOR_LINE 	2
#define EMU_CURSOR_NONE 	3

#define EMU_CHARSET_ASCII	0
#define EMU_CHARSET_UK		1
#define EMU_CHARSET_SPECIAL 2

#define EMU_EVENT_CONNECTED 	1
#define EMU_EVENT_DISCONNECTED	2
#define EMU_EVENT_CONNECTING	3

// 8 bits just ain't enough anymore.  Going to bit fields to handle
// things like text marking, blinking, underlining, etc.  Also can
// handle more colors this way if we want.	For now though stick to
// original scheme of 4 bits for foreground color and 4 bits for
// background color.

struct stAttribute
	{
	unsigned int txtclr : 4;		// text or foreground color index.
	unsigned int bkclr	: 4;		// background color index.
	unsigned int txtmrk : 1;		// true if text is 'marked'.
	unsigned int undrln : 1;		// underline
	unsigned int hilite : 1;		// foreground intensity
	unsigned int bklite : 1;		// background intensity
	unsigned int blink	: 1;		// soon to be famous blink attribute
	unsigned int revvid : 1;		// reverse video
	unsigned int blank	: 1;		// blank attribute
	unsigned int dblwilf: 1;		// double wide left
	unsigned int dblwirt: 1;		// double wide right
	unsigned int dblhilo: 1;		// double height top half
	unsigned int dblhihi: 1;		// double height bottom half
	unsigned int symbol:  1;		// use symbol font
	unsigned int wilf	: 1;		// wide left
	unsigned int wirt	: 1;		// wide right
	};

typedef struct stAttribute STATTR;
typedef STATTR *PSTATTR;

struct emuSettings
	{
	int 	nEmuId, 			// 100 = EMU_AUTO
								// 101 = EMU_ANSI
								// 102 = EMU_MINI
								// 109 = EMU_VIEW
								// 110 = EMU_TTY
								// 111 = EMU_VT100
								// 115 = EMU_VT52
								// 116 = EMU_VT100J
								//
			nTermKeys,			// 0 = EMU_KEYS_ACCEL
								// 1 = EMU_KEYS_TERM
								// 2 = EMU_KEYS_SCAN
								//
			nCursorType,		// 1 = EMU_CURSOR_BLOCK
								// 2 = EMU_CURSOR_LINE
								// 3 = EMU_CURSOR_NONE
								//
			nCharacterSet,		// 0 = EMU_CHARSET_ASCII
								// 1 = EMU_CHARSET_UK
								// 2 = EMU_CHARSET_SPECIAL
								//
			nAutoAttempts,		// Count of connections using the Auto
								// Detect Emulator.  At
								// EMU_MAX_AUTODETECT_ATTEMPTS, we switch
								// to Ansi emulation.  Note, this may
								// get moved into a Statictics Handle
								// if we ever develop one.
								//
			fCursorBlink,		// Blinking cursor. 			True\False.
			fMapPFkeys, 		// PF1-PF4 to top row of keypad.True\False.
			fAltKeypadMode, 	// Alternate keypad mode.		True\False.
			fKeypadAppMode, 	// Keypad application mode. 	True\False.
			fCursorKeypadMode,	// Cursor keypad mode.			True\Fales.
			fReverseDelBk,		// Reverse Del and Backsp.		True\False.
			f132Columns,		// 132 column display.			True\False.
			fDestructiveBk, 	// Destructive backspace.		True\False.
			fWrapLines, 		// Wrap lines.					True\False.
			fLbSymbolOnEnter;	// Send # symbol on Enter.		True\False.
	};

typedef struct emuSettings STEMUSET;
typedef STEMUSET *PSTEMUSET;

/* emuhdl.c */
HEMU	emuCreateHdl(const HSESSION hSession);
int 	emuDestroyHdl(const HEMU hEmu);
int 	emuLoad(const HEMU hEmu, const int nEmuId);
void	emuLock(const HEMU hEmu);
void	emuUnlock(const HEMU hEmu);
ECHAR	**emuGetTxtBuf(const HEMU hEmu);
PSTATTR *emuGetAttrBuf(const HEMU hEmu);
int 	emuKbdIn(const HEMU hEmu, KEY_T key, const int fTest);
int 	emuDataIn(const HEMU hEmu, const ECHAR ccode);
int 	emuComDone(const HEMU hEmu);
int 	emuTrackingNotify(const HEMU hEmu);
int 	emuIsEmuKey(const HEMU hEmu, KEY_T key);
int 	emuQueryClearAttr(const HEMU hemu, PSTATTR pstClearAttr);
int 	emuQueryCurPos(const HEMU hEmu, int *row, int *col);
HPRINT	emuQueryPrintEchoHdl(const HEMU hEmu);
int 	emuQueryRowsCols(const HEMU hEmu, int *piRows, int *piCols);
int 	emuQueryEmulatorId(const HEMU hEmulator);
int 	emuNotify(const HEMU hEmu, const int nEvent);
int 	emuQueryCursorType(const HEMU hEmu);
int 	emuQueryName(const HEMU hEmu, TCHAR *achBuffer, int nSize);
int 	emuSetSettings(const HEMU hEmu, const PSTEMUSET pstSettings);
int 	emuQuerySettings(const HEMU hEmu, PSTEMUSET pstSettings);
int 	emuInitializeHdl(const HEMU hEmu);
int 	emuSaveHdl(const HEMU hEmu);
int 	emuHomeHostCursor(const HEMU hEmu);
int 	emuEraseTerminalScreen(const HEMU hEmu);

void	emuMinitelSendKey(const HEMU hEmu, const int iCmd); // minitel.c
int 	emuGetIdFromName(const HEMU hEmu, TCHAR *achEmuName);

/*	colors indexes */

#define VC_BLACK		0
#define VC_BLUE 		1
#define VC_GREEN		2
#define VC_CYAN 		3
#define VC_RED			4
#define VC_MAGENTA		5
#define VC_BROWN		6
#define VC_WHITE		7
#define VC_GRAY 		8
#define VC_BRT_BLUE 	9
#define VC_BRT_GREEN	10
#define VC_BRT_CYAN 	11
#define VC_BRT_RED		12
#define VC_BRT_MAGENTA	13
#define VC_BRT_YELLOW	14
#define VC_BRT_WHITE	15
