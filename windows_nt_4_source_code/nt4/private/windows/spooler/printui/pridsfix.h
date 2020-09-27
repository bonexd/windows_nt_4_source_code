/*++

Copyright (c) 1995  Microsoft Corporation
All rights reserved.

Module Name:

    prids.h

Abstract:

    Printer Ids.

Author:

    Albert Ting (AlbertT)  22-Jun-1995

Revision History:

--*/

/********************************************************************

    Header translations: 0x3900 -> 0x3a00

    These strings are used in the column headers of the Queue UI.
    Although all of the PRINTER_NOTIFY_* fields are defined here,
    only those that actually visible in the UI are defined in
    the resource file.

********************************************************************/

#define IDS_HEAD            0x3900
#define IDS_HEAD_DELTA      0x80

#define IDS_HEAD_JOB                         IDS_HEAD
#define IDS_HEAD_JOB_PRINTER_NAME            (IDS_HEAD_JOB + JOB_NOTIFY_FIELD_PRINTER_NAME)
#define IDS_HEAD_JOB_MACHINE_NAME            (IDS_HEAD_JOB + JOB_NOTIFY_FIELD_MACHINE_NAME)
#define IDS_HEAD_JOB_PORT_NAME               (IDS_HEAD_JOB + JOB_NOTIFY_FIELD_PORT_NAME)
#define IDS_HEAD_JOB_USER_NAME               (IDS_HEAD_JOB + JOB_NOTIFY_FIELD_USER_NAME)
#define IDS_HEAD_JOB_NOTIFY_NAME             (IDS_HEAD_JOB + JOB_NOTIFY_FIELD_NOTIFY_NAME)
#define IDS_HEAD_JOB_DATATYPE                (IDS_HEAD_JOB + JOB_NOTIFY_FIELD_DATATYPE)
#define IDS_HEAD_JOB_PRINT_PROCESSOR         (IDS_HEAD_JOB + JOB_NOTIFY_FIELD_PRINT_PROCESSOR)
#define IDS_HEAD_JOB_PARAMETERS              (IDS_HEAD_JOB + JOB_NOTIFY_FIELD_PARAMETERS)
#define IDS_HEAD_JOB_DRIVER_NAME             (IDS_HEAD_JOB + JOB_NOTIFY_FIELD_DRIVER_NAME)
#define IDS_HEAD_JOB_DEVMODE                 (IDS_HEAD_JOB + JOB_NOTIFY_FIELD_DEVMODE)
#define IDS_HEAD_JOB_STATUS                  (IDS_HEAD_JOB + JOB_NOTIFY_FIELD_STATUS)
#define IDS_HEAD_JOB_STATUS_STRING           (IDS_HEAD_JOB + JOB_NOTIFY_FIELD_STATUS_STRING)
#define IDS_HEAD_JOB_SECURITY_DESCRIPTOR     (IDS_HEAD_JOB + JOB_NOTIFY_FIELD_SECURITY_DESCRIPTOR)
#define IDS_HEAD_JOB_DOCUMENT                (IDS_HEAD_JOB + JOB_NOTIFY_FIELD_DOCUMENT)
#define IDS_HEAD_JOB_PRIORITY                (IDS_HEAD_JOB + JOB_NOTIFY_FIELD_PRIORITY)
#define IDS_HEAD_JOB_POSITION                (IDS_HEAD_JOB + JOB_NOTIFY_FIELD_POSITION)
#define IDS_HEAD_JOB_SUBMITTED               (IDS_HEAD_JOB + JOB_NOTIFY_FIELD_SUBMITTED)
#define IDS_HEAD_JOB_START_TIME              (IDS_HEAD_JOB + JOB_NOTIFY_FIELD_START_TIME)
#define IDS_HEAD_JOB_UNTIL_TIME              (IDS_HEAD_JOB + JOB_NOTIFY_FIELD_UNTIL_TIME)
#define IDS_HEAD_JOB_TIME                    (IDS_HEAD_JOB + JOB_NOTIFY_FIELD_TIME)
#define IDS_HEAD_JOB_TOTAL_PAGES             (IDS_HEAD_JOB + JOB_NOTIFY_FIELD_TOTAL_PAGES)
#define IDS_HEAD_JOB_PAGES_PRINTED           (IDS_HEAD_JOB + JOB_NOTIFY_FIELD_PAGES_PRINTED)
#define IDS_HEAD_JOB_TOTAL_BYTES             (IDS_HEAD_JOB + JOB_NOTIFY_FIELD_TOTAL_BYTES)
#define IDS_HEAD_JOB_BYTES_PRINTED           (IDS_HEAD_JOB + JOB_NOTIFY_FIELD_BYTES_PRINTED)

#define IDS_HEAD_PRINTER                     (IDS_HEAD_JOB + IDS_HEAD_DELTA)
#define IDS_HEAD_PRINTER_SERVER_NAME         (IDS_HEAD_PRINTER + PRINTER_NOTIFY_FIELD_SERVER_NAME)
#define IDS_HEAD_PRINTER_PRINTER_NAME        (IDS_HEAD_PRINTER + PRINTER_NOTIFY_FIELD_PRINTER_NAME)
#define IDS_HEAD_PRINTER_SHARE_NAME          (IDS_HEAD_PRINTER + PRINTER_NOTIFY_FIELD_SHARE_NAME)
#define IDS_HEAD_PRINTER_PORT_NAME           (IDS_HEAD_PRINTER + PRINTER_NOTIFY_FIELD_PORT_NAME)
#define IDS_HEAD_PRINTER_DRIVER_NAME         (IDS_HEAD_PRINTER + PRINTER_NOTIFY_FIELD_DRIVER_NAME)
#define IDS_HEAD_PRINTER_COMMENT             (IDS_HEAD_PRINTER + PRINTER_NOTIFY_FIELD_COMMENT)
#define IDS_HEAD_PRINTER_LOCATION            (IDS_HEAD_PRINTER + PRINTER_NOTIFY_FIELD_LOCATION)
#define IDS_HEAD_PRINTER_DEVMODE             (IDS_HEAD_PRINTER + PRINTER_NOTIFY_FIELD_DEVMODE)
#define IDS_HEAD_PRINTER_SEPFILE             (IDS_HEAD_PRINTER + PRINTER_NOTIFY_FIELD_SEPFILE)
#define IDS_HEAD_PRINTER_PRINT_PROCESSOR     (IDS_HEAD_PRINTER + PRINTER_NOTIFY_FIELD_PRINT_PROCESSOR)
#define IDS_HEAD_PRINTER_PARAMETERS          (IDS_HEAD_PRINTER + PRINTER_NOTIFY_FIELD_PARAMETERS)
#define IDS_HEAD_PRINTER_DATATYPE            (IDS_HEAD_PRINTER + PRINTER_NOTIFY_FIELD_DATATYPE)
#define IDS_HEAD_PRINTER_SECURITY_DESCRIPTOR (IDS_HEAD_PRINTER + PRINTER_NOTIFY_FIELD_SECURITY_DESCRIPTOR)
#define IDS_HEAD_PRINTER_ATTRIBUTES          (IDS_HEAD_PRINTER + PRINTER_NOTIFY_FIELD_ATTRIBUTES)
#define IDS_HEAD_PRINTER_PRIORITY            (IDS_HEAD_PRINTER + PRINTER_NOTIFY_FIELD_PRIORITY)
#define IDS_HEAD_PRINTER_DEFAULT_PRIORITY    (IDS_HEAD_PRINTER + PRINTER_NOTIFY_FIELD_DEFAULT_PRIORITY)
#define IDS_HEAD_PRINTER_START_TIME          (IDS_HEAD_PRINTER + PRINTER_NOTIFY_FIELD_START_TIME)
#define IDS_HEAD_PRINTER_UNTIL_TIME          (IDS_HEAD_PRINTER + PRINTER_NOTIFY_FIELD_UNTIL_TIME)
#define IDS_HEAD_PRINTER_STATUS              (IDS_HEAD_PRINTER + PRINTER_NOTIFY_FIELD_STATUS)
#define IDS_HEAD_PRINTER_STATUS_STRING       (IDS_HEAD_PRINTER + PRINTER_NOTIFY_FIELD_STATUS_STRING)
#define IDS_HEAD_PRINTER_CJOBS               (IDS_HEAD_PRINTER + PRINTER_NOTIFY_FIELD_CJOBS)
#define IDS_HEAD_PRINTER_AVERAGE_PPM         (IDS_HEAD_PRINTER + PRINTER_NOTIFY_FIELD_AVERAGE_PPM)
#define IDS_HEAD_PRINTER_TOTAL_PAGES         (IDS_HEAD_PRINTER + PRINTER_NOTIFY_FIELD_TOTAL_PAGES)
#define IDS_HEAD_PRINTER_PAGES_PRINTED       (IDS_HEAD_PRINTER + PRINTER_NOTIFY_FIELD_PAGES_PRINTED)
#define IDS_HEAD_PRINTER_TOTAL_BYTES         (IDS_HEAD_PRINTER + PRINTER_NOTIFY_FIELD_TOTAL_BYTES)
#define IDS_HEAD_PRINTER_BYTES_PRINTED       (IDS_HEAD_PRINTER + PRINTER_NOTIFY_FIELD_BYTES_PRINTED)

#define IDS_DRIVER_BASE                 0x3a20
#define IDS_DRIVER_ALPHA_0              (IDS_DRIVER_BASE + DRIVER_ALPHA_0)
#define IDS_DRIVER_X86_0                (IDS_DRIVER_BASE + DRIVER_X86_0)
#define IDS_DRIVER_MIPS_0               (IDS_DRIVER_BASE + DRIVER_MIPS_0)
#define IDS_DRIVER_WIN95                (IDS_DRIVER_BASE + DRIVER_WIN95)
#define IDS_DRIVER_ALPHA_1              (IDS_DRIVER_BASE + DRIVER_ALPHA_1)
#define IDS_DRIVER_X86_1                (IDS_DRIVER_BASE + DRIVER_X86_1)
#define IDS_DRIVER_MIPS_1               (IDS_DRIVER_BASE + DRIVER_MIPS_1)
#define IDS_DRIVER_PPC_1                (IDS_DRIVER_BASE + DRIVER_PPC_1)
#define IDS_DRIVER_ALPHA_2              (IDS_DRIVER_BASE + DRIVER_ALPHA_2)
#define IDS_DRIVER_X86_2                (IDS_DRIVER_BASE + DRIVER_X86_2)
#define IDS_DRIVER_MIPS_2               (IDS_DRIVER_BASE + DRIVER_MIPS_2)
#define IDS_DRIVER_PPC_2                (IDS_DRIVER_BASE + DRIVER_PPC_2)

#define IDS_DRIVER_END                  0x3a3e

#define IDS_MH_PRINTER_PAUSE                (MH_PRINTER + IDM_PRINTER_PAUSE)
#define IDS_MH_PRINTER_INSTALL              (MH_PRINTER + IDM_PRINTER_INSTALL)
#define IDS_MH_PRINTER_PURGE                (MH_PRINTER + IDM_PRINTER_PURGE)
#define IDS_MH_PRINTER_SET_DEFAULT          (MH_PRINTER + IDM_PRINTER_SET_DEFAULT)
#define IDS_MH_PRINTER_SHARING              (MH_PRINTER + IDM_PRINTER_SHARING)
#define IDS_MH_REFRESH                      (MH_PRINTER + IDM_REFRESH)
#define IDS_MH_PRINTER_PROPERTIES           (MH_PRINTER + IDM_PRINTER_PROPERTIES)
#define IDS_MH_PRINTER_CLOSE                (MH_PRINTER + IDM_PRINTER_CLOSE)
#define IDS_MH_PRINTER_DOCUMENT_DEFAULTS    (MH_PRINTER + IDM_PRINTER_DOCUMENT_DEFAULTS)
#define IDS_MH_STATUS_BAR                   (MH_PRINTER + IDM_STATUS_BAR)
#define IDS_MH_HELP_CONTENTS                (MH_PRINTER + IDM_HELP_CONTENTS)
#define IDS_MH_HELP_ABOUT                   (MH_PRINTER + IDM_HELP_ABOUT)
#define IDS_MH_JOB_PAUSE                    (MH_PRINTER + IDM_JOB_PAUSE)
#define IDS_MH_JOB_RESUME                   (MH_PRINTER + IDM_JOB_RESUME)
#define IDS_MH_JOB_RESTART                  (MH_PRINTER + IDM_JOB_RESTART)
#define IDS_MH_JOB_CANCEL                   (MH_PRINTER + IDM_JOB_CANCEL)
#define IDS_MH_JOB_PROPERTIES               (MH_PRINTER + IDM_JOB_PROPERTIES)



