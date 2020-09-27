/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    WOW.C

Abstract:

    This file contains the code which is responsable for dealing with
    the WOW debug structures and debug events.

Author:

    James L Schaad (jimsch) 05-11-92

Environment:

    User Mode WIN32

--*/


#include "precomp.h"
#pragma hdrstop




/************************* GLOBAL VARIABLES ******************************/

extern char abEMReplyBuf[];
extern DDVECTOR RgfnFuncEventDispatch[];
extern DDVECTOR DebugDispatchTable[];
extern char nameBuffer[];

/********************** Local Prototypes *********************************/

int  LookupDllName(HPRCX hprcx, char * sz);
int  InsertDllName(HPRCX hprcx, char * sz);
void RemoveDllName(HPRCX hprcx, int idx);

static  DEBUG_EVENT     DeWow;


/************************** FUNCTIONS ************************************/


void
ProcessEntryPointEvent(
    DEBUG_EVENT *   pde,
    HTHDX           hthdx
    )
/*++

Routine Description:

    This handles task start events after the first one.

Arguments:


Return Value:


--*/
{
#if defined(i386) && !defined(WIN32S)
    IMAGE_NOTE  in;
    LPBYTE      lpb;
    DWORD       cbRead;
    int         b;
    char        szName0[_MAX_FNAME + _MAX_EXT];
    char        szName1[_MAX_FNAME + _MAX_EXT];
    char        szExt  [_MAX_EXT];


    lpb = (LPBYTE)pde->
            u.Exception.ExceptionRecord.ExceptionInformation[2];
    b = DbgReadMemory(hthdx->hprc,
                      lpb,
                      &in,
                      sizeof(in),
                      &cbRead);

    if ((b == 0) || (cbRead != sizeof(in))) {
        b = GetLastError();
    } else {
        _splitpath(in.FileName, NULL, NULL, szName0, szExt);
        strcat(szName0, szExt);
        _splitpath(nameBuffer, NULL, NULL, szName1, szExt);
        strcat(szName1, szExt);

        if (_strcmpi(szName0, szName1) == 0) {
            *nameBuffer = 0;
            hthdx->hprc->pstate &= ~ps_preEntry;
            hthdx->tstate |= ts_stopped;
            NotifyEM(pde, hthdx, 0, (LPVOID)ENTRY_BP);
            return;
        }
    }

    AddQueue( QT_CONTINUE_DEBUG_EVENT,
              hthdx->hprc->pid,
              hthdx->tid,
              DBG_CONTINUE,
              0);
    hthdx->tstate &= ~(ts_stopped|ts_first|ts_second);
    hthdx->tstate |= ts_running;
    return;
#endif
}



void
ProcessSegmentLoadEvent(
                        DEBUG_EVENT *   pde,
                        HTHDX           hthdx
                        )

/*++

Routine Description:

    This function takes care of dealing with segment load events from
    the wow system.  These come in as exceptions and are translated
    to segment load events in ProcessDebugEvent.

Arguments:

    pde         - Supplies a pointer to the modified debug event
    hthdx       - Supplies the handle to the thread of the debug event

Return Value:

    None.

--*/

{
#if defined(i386) && !defined(WIN32S)

    LPDWORD     lpdw = &pde->u.Exception.ExceptionRecord.ExceptionInformation[0];
    int         mode = LOWORD( lpdw[0] );
    int         cb;
    int         cbRead;
    int         b;
    char        rgch[512];
    char *      lpb;
    LPMODULELOAD lpmdl;
    RTP         rtp;
    LPRTP       lprtp;
    WORD        packetType = tlfDebugPacket;
    HEMI        hemi;
    HPRCX       hprcx = hthdx->hprc;
    int         idx;
    SEGMENT_NOTE        sn;
    ADDR                addr;
    EXPECTED_EVENT *    pee;
    DWORD       eventCode;
    DWORD       subClass;
    LDT_ENTRY   ldt;
    BREAKPOINT *bp;


#ifdef i386
    DeWow = *pde;

    rtp.hpid = hprcx->hpid;
    rtp.htid = hthdx->htid;

    hthdx->fWowEvent   = TRUE;

    VDMProcessException(pde);

    switch ( mode ) {
        /*
         *   SEG LOAD:
         *
         *      LOWORD(lpdw[0]) --- DBG_SEGLOAD
         *      HIWORD(lpdw[0]) --- Unused
         *      LOWORD(lpdw[1]) --- Unused
         *      HIWORD(lpdw[1]) --- Unused
         *      lpdw[2]         --- pointer to SEGMENT_NOTE structure
         *      lpdw[3]         --- Reserved
         */

    case DBG_SEGLOAD:
        lpb = (char *) lpdw[2];
        b = DbgReadMemory(hprcx, lpb, &sn, sizeof(sn), &cbRead);

        if ((b == 0) || (cbRead != sizeof(sn))) {
            b = GetLastError();
            AddQueue( QT_CONTINUE_DEBUG_EVENT,
                      hthdx->hprc->pid,
                      hthdx->tid,
                      DBG_CONTINUE,
                      0);
            hthdx->tstate &= ~(ts_stopped|ts_first|ts_second);
            hthdx->tstate |= ts_running;
            return;
        }

        if (sn.FileName[0] == 0) {
            AddQueue( QT_CONTINUE_DEBUG_EVENT,
                      hthdx->hprc->pid,
                      hthdx->tid,
                      DBG_CONTINUE,
                      0);
            hthdx->tstate &= ~(ts_stopped|ts_first|ts_second);
            hthdx->tstate |= ts_running;
            return;
        }
        cb = strlen(sn.FileName)+1;

        idx = LookupDllName(hprcx, sn.FileName);

        if ( idx != -1 ) {
            if (hprcx->rgDllList[idx].fReal) {
                //
                //  Changing from real to protected mode. We don't
                //  support this, so we'll throw away what we have
                //  and start from scratch.
                //
                lprtp = (LPRTP) rgch;
                *lprtp = rtp;
                lprtp->dbc = dbceModFree16;
                lprtp->cb = sizeof(WORD);
                *((WORD *)(&lprtp->rgbVar[0])) = idx;

                DmTlFunc(tlfDebugPacket, rtp.hpid, lprtp->cb + sizeof(RTP),
                         (LONG)(LPV) lprtp);

                RemoveDllName( hprcx, idx );
                idx = -1;
            }
        }

        if (idx == -1) {

            cb = cb + sizeof(MODULELOAD) + (sn.Segment+1)*sizeof(OBJD);
            lprtp = (LPRTP) malloc(cb + sizeof(RTP));
            memset(lprtp, 0, cb + sizeof(RTP));

            *lprtp = rtp;
            lprtp->dbc = dbcModLoad;
            lprtp->cb = cb;

            lpmdl = (LPMODULELOAD) &(lprtp->rgbVar[0]);
            lpmdl->cobj = sn.Segment+1;

            lpmdl->rgobjd[sn.Segment].wSel = sn.Selector1;
            lpmdl->rgobjd[sn.Segment].cb = (DWORD) -1;
            lpmdl->rgobjd[sn.Segment].wPad = 1;

            lpmdl->mte = InsertDllName(hprcx, sn.FileName);
            strcpy((char *) &lpmdl->rgobjd[lpmdl->cobj], sn.FileName);

            lpmdl->fRealMode = FALSE;
            lpmdl->fFlatMode = FALSE;
            lpmdl->fOffset32 = FALSE;

            DmTlFunc(tlfRequest, rtp.hpid, lprtp->cb + sizeof(RTP),
                     (LONG)(LPV) lprtp);

            hemi = *((HEMI *) abEMReplyBuf);

            free(lprtp);
            AddQueue( QT_CONTINUE_DEBUG_EVENT,
                      hthdx->hprc->pid,
                      hthdx->tid,
                      DBG_CONTINUE,
                      0);
            hthdx->tstate &= ~(ts_stopped|ts_first|ts_second);
            hthdx->tstate |= ts_running;

        } else {
            LPSLI       lpsli;

            lprtp = (LPRTP) malloc( sizeof(RTP) + sizeof(SLI));
            *lprtp = rtp;
            lprtp->dbc = dbceSegLoad;
            lprtp->cb = sizeof(SLI);
            lpsli = (LPSLI) &lprtp->rgbVar[0];
            lpsli->wSelector = sn.Selector1;
            lpsli->wSegNo = sn.Segment;
            lpsli->mte = idx;

            DmTlFunc(tlfDebugPacket, rtp.hpid, lprtp->cb + sizeof(RTP),
                     (LONG)(LPV) lprtp);

            free( lprtp );
        }

        break;

        /*
         *   SEGMOVE:
         *
         *      This event will be triggered if a selector number
         *      is to be changed.
         *
         *      LOWORD( lpdw[0] ) - SEGMOVE
         *      LOWORD( lpdw[1] ) - old selector number
         *      HIWORD( lpdw[1] ) - new selector number
         */

    case DBG_SEGMOVE:

        lpb = (char *) lpdw[2];
        b = DbgReadMemory(hprcx, lpb, &sn, sizeof(sn), &cbRead);

        if ((b == 0) || (cbRead != sizeof(sn))) {
            b = GetLastError();
            AddQueue( QT_CONTINUE_DEBUG_EVENT,
                      hthdx->hprc->pid,
                      hthdx->tid,
                      DBG_CONTINUE,
                      0);
            hthdx->tstate &= ~(ts_stopped|ts_first|ts_second);
            hthdx->tstate |= ts_running;
            return;
        }

        if (sn.FileName[0] == 0) {
            AddQueue( QT_CONTINUE_DEBUG_EVENT,
                      hthdx->hprc->pid,
                      hthdx->tid,
                      DBG_CONTINUE,
                      0);
            hthdx->tstate &= ~(ts_stopped|ts_first|ts_second);
            hthdx->tstate |= ts_running;
            return;
        }
        cb = strlen(sn.FileName)+1;

        idx = LookupDllName(hprcx, sn.FileName);

        if ( idx != -1 ) {
            LPSLI       lpsli;

            lprtp = (LPRTP) malloc( sizeof(RTP) + sizeof(SLI));
            *lprtp = rtp;
            lprtp->dbc = dbceSegMove;
            lprtp->cb = sizeof(SLI);
            lpsli = (LPSLI) &lprtp->rgbVar[0];
            assert( sn.Selector1 == 0 );
            lpsli->wSelector = sn.Selector2;
            lpsli->wSegNo = sn.Segment;
            lpsli->mte = idx;

            DmTlFunc(tlfDebugPacket, rtp.hpid, lprtp->cb + sizeof(RTP),
                     (LONG)(LPV) lprtp);

            free( lprtp );
        }

        AddQueue( QT_CONTINUE_DEBUG_EVENT,
                  hthdx->hprc->pid,
                  hthdx->tid,
                  DBG_CONTINUE,
                  0);
        hthdx->tstate &= ~(ts_stopped|ts_first|ts_second);
        hthdx->tstate |= ts_running;

        break;

        /*
         *   SEGFREE:
         *
         *      This event is triggered if a selector is freed
         *
         *      LOWORD( lpdw[0] ) - SEGFREE
         *      HIWORD( lpdw[0] ) - fBPRelease
         *      LOWORD( lpdw[1] ) - selector to be freed
         */

    case DBG_SEGFREE:

        AddQueue( QT_CONTINUE_DEBUG_EVENT,
                  hthdx->hprc->pid,
                  hthdx->tid,
                  DBG_CONTINUE,
                  0);
        hthdx->tstate &= ~(ts_stopped|ts_first|ts_second);
        hthdx->tstate |= ts_running;

        break;

        /*
         *   MODLOAD:
         *
         *      This event is triggered when a new DLL is loaded
         *
         *      LOWORD( lpdw[0] ) - MODLOAD
         *      HIWORD( lpdw[0] ) - length of module name
         *      HIWORD( lpdw[1] ) - selector
         *      lpdw[2]           - address of module name
         *      lpdw[3]           - image length
         *
         */

    case DBG_MODLOAD:

        lpb = (char *) lpdw[2];
        b = DbgReadMemory(hprcx, lpb, &sn, sizeof(sn), &cbRead);

        if ((b == 0) || (cbRead != sizeof(sn))) {
            b = GetLastError();
            AddQueue( QT_CONTINUE_DEBUG_EVENT,
                      hthdx->hprc->pid,
                      hthdx->tid,
                      DBG_CONTINUE,
                      0);
            hthdx->tstate &= ~(ts_stopped|ts_first|ts_second);
            hthdx->tstate |= ts_running;
            return;
        }

        if (sn.FileName[0] == 0) {
            AddQueue( QT_CONTINUE_DEBUG_EVENT,
                      hthdx->hprc->pid,
                      hthdx->tid,
                      DBG_CONTINUE,
                      0);
            hthdx->tstate &= ~(ts_stopped|ts_first|ts_second);
            hthdx->tstate |= ts_running;
            return;
        }

        cb = strlen(sn.FileName)+1;
        idx = LookupDllName(hprcx, sn.FileName);

        if (idx == -1) {
            cb = cb + sizeof(MODULELOAD);
            lprtp = (LPRTP) malloc(cb + sizeof(RTP));
            memset(lprtp, 0, cb + sizeof(RTP));

            *lprtp = rtp;
            lprtp->dbc = dbcModLoad;
            lprtp->cb = cb;

            lpmdl = (LPMODULELOAD) &(lprtp->rgbVar[0]);
            lpmdl->cobj = 0;

            lpmdl->mte = InsertDllName(hprcx, sn.FileName);

            idx = LookupDllName(hprcx, sn.FileName);

            if ( idx != -1 ) {
                hprcx->rgDllList[idx].fReal = TRUE;
            }

            strcpy((char *) &lpmdl->rgobjd[lpmdl->cobj], sn.FileName);

            lpmdl->StartingSegment = sn.Segment;

            lpmdl->fRealMode = TRUE;
            lpmdl->fFlatMode = FALSE;
            lpmdl->fOffset32 = FALSE;

            DmTlFunc(tlfRequest, rtp.hpid, lprtp->cb + sizeof(RTP),
                     (LONG)(LPV) lprtp);

            hemi = *((HEMI *) abEMReplyBuf);

            free(lprtp);
        }

        AddQueue( QT_CONTINUE_DEBUG_EVENT,
                  hthdx->hprc->pid,
                  hthdx->tid,
                  DBG_CONTINUE,
                  0);
        hthdx->tstate &= ~(ts_stopped|ts_first|ts_second);
        hthdx->tstate |= ts_running;

        break;

        /*
         *   MODFREE:
         *
         *      This event is triggered when a DLL is unloaded
         *
         *      LOWORD( lpdw[0] ) - MODFREE
         */

    case DBG_MODFREE:
        lpb = (char *) lpdw[2];
        b = DbgReadMemory(hprcx, lpb, &sn, sizeof(sn), &cbRead);

        if ((b == 0) || (cbRead != sizeof(sn))) {
            b = GetLastError();
            AddQueue( QT_CONTINUE_DEBUG_EVENT,
                      hthdx->hprc->pid,
                      hthdx->tid,
                      DBG_CONTINUE,
                      0);
            hthdx->tstate &= ~(ts_stopped|ts_first|ts_second);
            hthdx->tstate |= ts_running;
            return;
        }

        if (sn.FileName[0] == 0) {
            AddQueue( QT_CONTINUE_DEBUG_EVENT,
                      hthdx->hprc->pid,
                      hthdx->tid,
                      DBG_CONTINUE,
                      0);
            hthdx->tstate &= ~(ts_stopped|ts_first|ts_second);
            hthdx->tstate |= ts_running;
            return;
        }
        cb = strlen(sn.FileName)+1;

        idx = LookupDllName(hprcx, sn.FileName);

        if (idx != -1) {
            lprtp = (LPRTP) rgch;
            *lprtp = rtp;
            lprtp->dbc = dbceModFree16;
            lprtp->cb = sizeof(WORD);
            *((WORD *)(&lprtp->rgbVar[0])) = idx;

            DmTlFunc(tlfDebugPacket, rtp.hpid, lprtp->cb + sizeof(RTP),
                     (LONG)(LPV) lprtp);

            RemoveDllName( hprcx, idx );
        }

        AddQueue( QT_CONTINUE_DEBUG_EVENT,
                  hthdx->hprc->pid,
                  hthdx->tid,
                  DBG_CONTINUE,
                  0);
        hthdx->tstate &= ~(ts_stopped|ts_first|ts_second);
        hthdx->tstate |= ts_running;

        break;

        /*
         *  Int 01h break;
         */

    case DBG_SINGLESTEP:
        hthdx->context.ContextFlags = VDMCONTEXT_FULL;
        VDMGetThreadContext(pde, &hthdx->context);

        eventCode = EXCEPTION_DEBUG_EVENT;
        pde->dwDebugEventCode = EXCEPTION_DEBUG_EVENT;
        pde->u.Exception.ExceptionRecord.ExceptionCode = subClass =
            (DWORD)EXCEPTION_SINGLE_STEP;

        /*
         *  They are not clearing the trace bit
         */

        hthdx->context.EFlags &= ~TF_BIT_MASK;
        hthdx->fContextDirty = TRUE;

        AddrInit(&addr, 0, (SEGMENT) hthdx->context.SegCs, hthdx->context.Eip,
                 FALSE, FALSE, FALSE, FALSE);

        bp = FindBP(hthdx->hprc, hthdx, bptpExec, (BPNS)-1, &addr, FALSE);

        if ( bp ) {
            SetBPFlag( hthdx, bp );
        }

        goto dispatch;

    case DBG_TASKSTART:
        hthdx->context.ContextFlags = VDMCONTEXT_FULL;
        VDMGetThreadContext(pde, &hthdx->context);

        eventCode =
        pde->dwDebugEventCode = ENTRYPOINT_DEBUG_EVENT;

        goto dispatch;

    case DBG_TASKSTOP:
    case DBG_DLLSTART:
    case DBG_DLLSTOP:
    case DBG_ATTACH:
        AddQueue( QT_CONTINUE_DEBUG_EVENT,
                  hthdx->hprc->pid,
                  hthdx->tid,
                  DBG_CONTINUE,
                  0);
        hthdx->tstate &= ~(ts_stopped|ts_first|ts_second);
        hthdx->tstate |= ts_running;
        break;


        /*
         *   Int 03h break
         *
         *      LOWORD(lpdw[0])  --- BREAKPOINT
         *      HIWORD(lpdw[0])  --- Protect Mode
         */

    case DBG_BREAK:
        hthdx->context.ContextFlags = VDMCONTEXT_FULL;
        VDMGetThreadContext(pde, &hthdx->context);

        PC(hthdx) -= 1;
        hthdx->fContextDirty = TRUE;

        eventCode = pde->dwDebugEventCode = BREAKPOINT_DEBUG_EVENT;

        // NOTENOTE --- jimsch -- assuming only 0xcc not 0xcd 0x3 breakpoints

        AddrInit(&addr, 0, (SEGMENT) hthdx->context.SegCs, hthdx->context.Eip,
                 FALSE, FALSE, FALSE, FALSE);

        bp = FindBP(hthdx->hprc, hthdx, bptpExec, (BPNS)-1, &addr, FALSE);

        if ( bp && bp->isStep ) {

            eventCode             = EXCEPTION_DEBUG_EVENT;
            pde->dwDebugEventCode = EXCEPTION_DEBUG_EVENT;

            pde->u.Exception.ExceptionRecord.ExceptionCode =
                subClass = (DWORD)EXCEPTION_SINGLE_STEP;
                RemoveBP(bp);

        } else {

            if ( bp ) {
                SetBPFlag( hthdx, bp );
            }

            pde->u.Exception.ExceptionRecord.ExceptionCode =
                subClass = (DWORD)bp;
        }

        pde->u.Exception.ExceptionRecord.ExceptionAddress = (LPVOID) PC(hthdx);

    dispatch:
        hthdx->fAddrIsReal = FALSE;
        hthdx->fAddrIsFlat = FALSE;

        if (VDMGetThreadSelectorEntry(pde,
                                      hthdx->rwHand,
                                      (WORD) hthdx->context.SegCs,
                                      &ldt)) {
            if (ldt.HighWord.Bits.Default_Big) {
                hthdx->fAddrOff32 = TRUE;
            } else {
                hthdx->fAddrOff32 = FALSE;
            }
        } else {
            hthdx->fAddrOff32 = FALSE;
        }

        /*
         *  Check if this debug event was expected
         */

        pee = PeeIsEventExpected(hthdx, eventCode, subClass);

        /*
         * If it wasn't, run the standard handler with
         * notifications going to the execution model
         */

        assert((0 < eventCode) && (eventCode < MAX_EVENT_CODE));

        if (pee == NULL) {
            if ((hthdx != NULL) && (hthdx->tstate & ts_funceval)) {
                RgfnFuncEventDispatch[eventCode-EXCEPTION_DEBUG_EVENT](pde, hthdx);
            } else {
                DebugDispatchTable[eventCode-EXCEPTION_DEBUG_EVENT](pde,hthdx);
            }
            return;
        }


        /*
         *  If it was expected then call the action
         * function if one was specified
         */

        if (pee->action) {
            (pee->action)(pde, hthdx, 0, pee->lparam);
        }

        /*
         *  And call the notifier if one was specified
         */

        if (pee->notifier) {
            METHOD  *nm = pee->notifier;
            (nm->notifyFunction)(pde, hthdx, 0, nm->lparam);
        }

        free(pee);
        break;

#if 0  // why is this here??
    case DBG_DIVOVERFLOW:
        pde->dwDebugEventCode = 3;
        goto fault_occured;

    case DBG_INSTRFAULT:
        pde->dwDebugEventCode = 1;
        goto fault_occured;
#endif

    case DBG_DIVOVERFLOW:
    case DBG_INSTRFAULT:
    case DBG_GPFAULT:
        pde->dwDebugEventCode = EXCEPTION_DEBUG_EVENT;

#if 0  // why is this here??
    fault_occured:
#endif

        hthdx->context.ContextFlags = VDMCONTEXT_FULL;
        VDMGetThreadContext(pde, &hthdx->context);
        pde->u.Exception.ExceptionRecord.ExceptionCode = 13;

        hthdx->fAddrIsReal = FALSE;
        hthdx->fAddrIsFlat = FALSE;

        if (VDMGetThreadSelectorEntry(pde, hthdx->rwHand,
                                      (WORD) hthdx->context.SegCs, &ldt)) {
            if (ldt.HighWord.Bits.Default_Big) {
                hthdx->fAddrOff32 = TRUE;
            } else {
                hthdx->fAddrOff32 = FALSE;
            }
        } else {
            hthdx->fAddrOff32 = FALSE;
        }

        ProcessExceptionEvent(pde, hthdx);
        break;

    default:
        AddQueue( QT_CONTINUE_DEBUG_EVENT,
                  hthdx->hprc->pid,
                  hthdx->tid,
                  DBG_CONTINUE,
                  0);
        hthdx->tstate &= ~(ts_stopped|ts_first|ts_second);
        hthdx->tstate |= ts_running;
        break;
    }
#endif // i386
#endif // i386 && !Win32S
    return;
}                               /* ProcessSegmentLoadEvent() */


#if defined(i386) && !defined(WIN32S)

int
LookupDllName(
              HPRCX     hprcx,
              char *    sz
              )

/*++

Routine Description:

    This routine will snoop through the structures of an exe and determine
    if we have previously seen this dll name for this process.

Arguments:

    hprcx   - Supplies a handle to a process
    sz      - Supplies a pointer to a DLL file name
    .

Return Value:

    The index in the array if present. Else -1

--*/

{
    int         i;

    /*
     *  If there is no list then the item is not on the list
     */

    if (hprcx->rgDllList == NULL) {
        return -1;
    }

    /*
     *  Check each item in the list
     */

    for (i=0; i<hprcx->cDllList; i++) {
        if ((hprcx->rgDllList[i].fValidDll) &&
            (hprcx->rgDllList[i].szDllName != NULL) &&
            (strcmp(hprcx->rgDllList[i].szDllName, sz) == 0)) {
            return i;
        }
    }
    return -1;
}                               /* LookupDllName() */


int
InsertDllName(
              HPRCX     hprcx,
              char *    sz
              )

/*++

Routine Description:

    This routine will take a dll name and an mte and put then into the
    list of dlls for the current process.

Arguments:

    hprcx   - Supplies a handle to a process
    sz      - Supplies a pointer to a DLL file name

Return Value:

    The index in the array where it was placed.

--*/

{
    int         i;
    int         cb;
    int         cItem;

    /*
     *  If there is no list then make a list
     */

    if (hprcx->rgDllList == NULL) {
        hprcx->rgDllList = (PDLLLOAD_ITEM) malloc(sizeof(DLLLOAD_ITEM) * 10);
        memset(hprcx->rgDllList, 0, sizeof(DLLLOAD_ITEM)*10);
    }

    /*
     *  Check each item in the list
     */

    for (i=0; i<hprcx->cDllList; i++) {
        if ((hprcx->rgDllList[i].fValidDll == FALSE) &&
            (hprcx->rgDllList[i].szDllName == NULL)) {
            hprcx->rgDllList[i].szDllName = _strdup(sz);
            hprcx->rgDllList[i].fValidDll = TRUE;
            hprcx->rgDllList[i].fWow = TRUE;
            return i;
        }
    }

    cItem = hprcx->cDllList;
    if (i == cItem) {
        cb = i * sizeof(DLLLOAD_ITEM);
        cItem += 10;
        hprcx->rgDllList = (PDLLLOAD_ITEM) realloc(hprcx->rgDllList, cItem*sizeof(DLLLOAD_ITEM));
        memset(cb + (char *) hprcx->rgDllList, 0, 10*sizeof(DLLLOAD_ITEM));
    }


    hprcx->rgDllList[i].szDllName = _strdup(sz);
    hprcx->rgDllList[i].fValidDll = TRUE;
    hprcx->rgDllList[i].fWow = TRUE;
    hprcx->cDllList += 10;

    return i;
}                               /* InsertDllName() */



void
RemoveDllName(
              HPRCX     hprcx,
              int       idx
              )

/*++

Routine Description:

    This routine will remove a dll from the internal list of known dlls.

Arguments:

    hprcx  - Supplies the handle to the current process
    idx    - supplies the index of the dll to be removed

Return Value:

    None.

--*/

{
    assert( hprcx->rgDllList != NULL );

    free(hprcx->rgDllList[idx].szDllName);
    hprcx->rgDllList[idx].szDllName = NULL;
    hprcx->rgDllList[idx].fValidDll = FALSE;
    hprcx->rgDllList[idx].fReal     = FALSE;
    hprcx->rgDllList[idx].fWow = FALSE;

    return;
}                               /* RemoveDllName() */


#endif // i386 && !WIN32S

/*++

Routine Description:

    Callback for VDMEnumProcessWOW

Arguments:

    See PROCESSENUMPROC in vdmdbg.h

Return Value:

    TRUE

--*/
BOOL WINAPI EnumCallBack(
                DWORD   dwProcessId,
                DWORD   dwAttributes,
                LPARAM  lpUserDefined
                )
{
    UNREFERENCED_PARAMETER( dwProcessId );
    UNREFERENCED_PARAMETER( dwAttributes );
    UNREFERENCED_PARAMETER( lpUserDefined );

    return TRUE;
}



BOOL
IsWOWPresent(
    VOID
    )

/*++

Routine Description:

    Determines if WOW is running

Arguments:

    None

Return Value:

    TRUE if WOW is running, FALSE otherwise

--*/

{
#if defined(i386) && !defined(WIN32S)
    return VDMEnumProcessWOW( (PROCESSENUMPROC)EnumCallBack, 0 );
#else
    return FALSE;
#endif
}



BOOL
TranslateAddress(
                 HPRCX  hprc,
                 HTHDX  hthd,
                 LPADDR lpaddr,
                 BOOL   f16ToFlat
                 )

/*++

Routine Description:

    This function is used to preform address translations from the segmented
    to the flat world and back again.

Arguments:

    hprc        - Supplies the handle to the current process
    hthd        - Supplies the handle to the thread for the address
    lpaddr      - Supplies the address to be translated
    f16ToFlat   - Supplies the direction the translation is to be made

Return Value:

    TRUE on success and FALSE on failure

--*/

{
#if defined(i386)
    LDT_ENTRY   ldt;
    ULONG       ul;

#ifdef WIN32S
    ADDR_IS_FLAT(*lpaddr) = TRUE;
    ADDR_IS_REAL(*lpaddr) = FALSE;
    return TRUE;
#endif

    /*
     *  Step 0.  If the address has already been mapped flat then return
     */

    if (ADDR_IS_FLAT(*lpaddr)) {
        return TRUE;
    }

    /*
     *  Step 1.  Is to find a stopped thread.  This is mainly for WOW support
     *          where the lack of a stopped thread is a serious thing,
     *          we can not do anything smartly with wow if this is the
     *          case.   We will currently only search for a single
     *          stopped thread cause the blasted operating system won't
     *          let us have more than one.
     */

    if (hthd == 0) {
        for (hthd = hprc->hthdChild; hthd != hthdxNull; hthd = hthd->next) {
            if (hthd->tstate & ts_stopped) {
                break;
            }
        }

        if (hthd == 0) {
            hthd = hprc->hthdChild;
        }
    }

    /*
     *  Must have a thread
     */

    if (hthd == NULL) {
        return FALSE;
    }

    /*
     *  Step 2. Depending on if the last event was WOW we need to
     *          either go in with a WOW remap or do the standard
     *          non-wow remap
     */

    if (hthd->fAddrIsFlat) {
        if (ADDR_IS_REAL( *lpaddr )) {
            ul = GetAddrSeg(*lpaddr) * 16 + GetAddrOff(*lpaddr);
            lpaddr->addr.off = ul;
        } else if (GetThreadSelectorEntry(hthd->rwHand, GetAddrSeg(*lpaddr),
                                          &ldt)) {
            ul = (ldt.HighWord.Bytes.BaseHi << 24) |
                 (ldt.HighWord.Bytes.BaseMid << 16) |
                 (ldt.BaseLow);
            lpaddr->addr.off += ul;
        } else {
            /*
             * Unrecognized selector
             */
            return FALSE;
        }
    } else {
#if defined(i386) && !defined(WIN32S)
        lpaddr->addr.off = VDMGetPointer(hprc->rwHand, hthd->rwHand,
                                         GetAddrSeg(*lpaddr),
                                         GetAddrOff(*lpaddr),
                                         !lpaddr->mode.fReal);
#else  // defined(i386) && !defined(WIN32S)
        assert(FALSE);
        return FALSE;
#endif // defined(i386) && !defined(WIN32S)
    }
#endif
    ADDR_IS_FLAT(*lpaddr) = TRUE;
    ADDR_IS_REAL(*lpaddr) = FALSE;
    return TRUE;
}                               /* TranslateAddress() */


BOOL
WOWGetThreadContext(
                    HTHDX     hthdx,
                    LPCONTEXT lpcxt
                    )

/*++

Routine Description:

    This routine is called to g the context of a WOW thread.  We have
    a current assumption that we will only have one WOW event at a time.

Arguments:

    hthdx       - supplies the handle to the thread to change the context of
    lpcxt       - supplies the new context.

Return Value:

    TRUE on success and FALSE on failure

--*/

{
#if defined(i386) && !defined(WIN32S)
    assert(hthdx->fWowEvent);

    if (hthdx->tid != DeWow.dwThreadId) {
        assert(FALSE);
        return FALSE;
    }

    return VDMGetThreadContext(&DeWow, lpcxt);
#else
    return FALSE;
#endif
}                               /* WOWGetThreadContext() */



BOOL
WOWSetThreadContext(
                    HTHDX     hthdx,
                    LPCONTEXT lpcxt
                    )

/*++

Routine Description:

    This routine is called to set the context of a WOW thread.  We have
    a current assumption that we will only have one WOW event at a time.

Arguments:

    hthdx       - supplies the handle to the thread to change the context of
    lpcxt       - supplies the new context.

Return Value:

    TRUE on success and FALSE on failure

--*/

{
#if defined(i386) && !defined(WIN32S)
    assert(hthdx->fWowEvent);

    if (hthdx->tid != DeWow.dwThreadId) {
        assert(FALSE);
        return FALSE;
    }

    return VDMSetThreadContext(&DeWow, lpcxt);
#else
    return FALSE;
#endif
}                               /* WOWSetThreadContext() */
