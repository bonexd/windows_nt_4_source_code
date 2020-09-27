/*****************************************************************************

    ftpt.c

*****************************************************************************/
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winsock.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <lm.h>
#include <ftpd.h>



/*****************************************************************************

    globals

*****************************************************************************/
UINT _cchNumberPadding;



/*****************************************************************************

    prototypes

*****************************************************************************/
INT _CRTAPI1 main( INT    cArgs,
                   char * pArgs[] );

VOID Usage();

VOID DoEnum( WCHAR * pszServer );

VOID DoQuery( WCHAR * pszServer );

VOID DoSet( WCHAR * pszServer,
            CHAR  * pszReadMask,
            CHAR  * pszWriteMask );

VOID DoNuke( WCHAR * pszServer,
             CHAR  * pszUserId );

VOID DoStats( WCHAR * pszServer );

VOID DoClear( WCHAR * pszServer );

CHAR * MakeCommaString( CHAR * pszNumber );

CHAR * MakeCommaNumber( DWORD  dwNumber  );



/*****************************************************************************

    main

*****************************************************************************/
INT _CRTAPI1 main( INT    cArgs,
                   char * pArgs[] )
{
    WCHAR   szServer[MAX_PATH];
    WCHAR * pszServer = NULL;
    INT     iArg;

    if( cArgs < 2 )
    {
        Usage();
        return 1;
    }

    iArg = 1;

    if( *pArgs[iArg] == '\\' )
    {
        wsprintfW( szServer, L"%S", pArgs[iArg++] );
        pszServer = szServer;
        cArgs--;
    }

    if( _stricmp( pArgs[iArg], "stats" ) == 0 )
    {
        if( cArgs != 2 )
        {
            printf( "use: ftpt stats\n" );
            return 1;
        }

        DoStats( pszServer );
    }
    else
    if( _stricmp( pArgs[iArg], "clear" ) == 0 )
    {
        if( cArgs != 2 )
        {
            printf( "use: ftpt clear\n" );
            return 1;
        }

        DoClear( pszServer );
    }
    else
    if( _stricmp( pArgs[iArg], "enum" ) == 0 )
    {
        if( cArgs != 2 )
        {
            printf( "use: ftpt enum\n" );
            return 1;
        }

        DoEnum( pszServer );
    }
    else
    if( _stricmp( pArgs[iArg], "query" ) == 0 )
    {
        if( cArgs != 2 )
        {
            printf( "use: ftpt query\n" );
            return 1;
        }

        DoQuery( pszServer );
    }
    else
    if( _stricmp( pArgs[iArg], "set" ) == 0 )
    {
        CHAR * pszReadMask;
        CHAR * pszWriteMask;

        if( cArgs != 4 )
        {
            printf( "use: ftpt set read_mask write_mask\n" );
            return 1;
        }

        pszReadMask  = pArgs[++iArg];
        pszWriteMask = pArgs[++iArg];

        DoSet( pszServer,
               pszReadMask,
               pszWriteMask );
    }
    else
    if( _stricmp( pArgs[iArg], "nuke" ) == 0 )
    {
        CHAR * pszUserId;

        if( cArgs != 3 )
        {
            printf( "use: ftpt nuke user_id\n" );
            return 1;
        }

        pszUserId = pArgs[++iArg];

        DoNuke( pszServer,
                pszUserId );
    }
    else
    {
        Usage();
        return 1;
    }

    return 0;

}   // main



/*****************************************************************************

    Usage

*****************************************************************************/
VOID Usage( VOID )
{
    printf( "use: ftpt [\\\\server] command [options]\n" );
    printf( "Valid commands are:\n" );
    printf( "        enum  - Enumerates connected users.\n" );
    printf( "        query - Queries volume security masks.\n" );
    printf( "        set   - Sets volume security masks.\n" );
    printf( "        nuke  - Disconnect a user.\n" );
    printf( "        stats - Display server statistics.\n" );
    printf( "        clear - Clear server statistics.\n" );

}   // Usage



/*****************************************************************************

    DoEnum

*****************************************************************************/
VOID DoEnum( WCHAR * pszServer )
{
    NET_API_STATUS   err;
    FTP_USER_INFO  * pUserInfo;
    DWORD            cEntries;

    printf( "Invoking I_FtpEnumerateUsers..." );

    err = I_FtpEnumerateUsers( pszServer,
                               &cEntries,
                               &pUserInfo );

    if( err != NERR_Success )
    {
        printf( "ERROR %lu (%08lX)\n", err, err );
    }
    else
    {
        printf( "done\n" );
        printf( "read %lu connected users\n", cEntries );

        while( cEntries-- )
        {
            IN_ADDR addr;

            addr.s_addr = (u_long)pUserInfo->inetHost;

            printf( "idUser     = %lu\n", pUserInfo->idUser     );
            printf( "pszUser    = %S\n",  pUserInfo->pszUser    );
            printf( "fAnonymous = %lu\n", pUserInfo->fAnonymous );
            printf( "inetHost   = %s\n",  inet_ntoa( addr )     );
            printf( "tConnect   = %lu\n", pUserInfo->tConnect   );
            printf( "\n" );

            pUserInfo++;
        }
    }

}   // DoEnum



/*****************************************************************************

    DoQuery

*****************************************************************************/
VOID DoQuery( WCHAR * pszServer )
{
    NET_API_STATUS   err;
    DWORD            maskReadAccess;
    DWORD            maskWriteAccess;

    printf( "Invoking I_FtpQueryVolumeSecurity..." );

    err = I_FtpQueryVolumeSecurity( pszServer,
                                    &maskReadAccess,
                                    &maskWriteAccess );

    if( err != NERR_Success )
    {
        printf( "ERROR %lu (%08lX)\n", err, err );
    }
    else
    {
        printf( "done\n" );
        printf( "maskReadAccess  = %08lX\n", maskReadAccess  );
        printf( "maskWriteAccess = %08lX\n", maskWriteAccess );
    }

}   // DoQuery



/*****************************************************************************

    DoSet

*****************************************************************************/
VOID DoSet( WCHAR * pszServer,
            CHAR  * pszReadMask,
            CHAR  * pszWriteMask )
{
    NET_API_STATUS   err;
    DWORD            maskReadAccess;
    DWORD            maskWriteAccess;

    maskReadAccess  = (DWORD)strtoul( pszReadMask,  NULL, 0 );
    maskWriteAccess = (DWORD)strtoul( pszWriteMask, NULL, 0 );

    printf( "Invoking I_FtpSetVolumeSecurity..." );

    err = I_FtpSetVolumeSecurity( pszServer,
                                  maskReadAccess,
                                  maskWriteAccess );

    if( err != NERR_Success )
    {
        printf( "ERROR %lu (%08lX)\n", err, err );
    }
    else
    {
        printf( "done\n" );
    }

}   // DoSet



/*****************************************************************************

    DoNuke

*****************************************************************************/
VOID DoNuke( WCHAR * pszServer,
             CHAR  * pszUserId )
{
    NET_API_STATUS   err;
    DWORD            idUser;

    idUser = (DWORD)strtoul( pszUserId, NULL, 0 );

    printf( "Invoking I_FtpDisconnectUser..." );

    err = I_FtpDisconnectUser( pszServer,
                               idUser );

    if( err != NERR_Success )
    {
        printf( "ERROR %lu (%08lX)\n", err, err );
    }
    else
    {
        printf( "done\n" );
    }

}   // DoNuke



/*****************************************************************************

    DoStats

*****************************************************************************/
VOID DoStats( WCHAR * pszServer )
{
    NET_API_STATUS    err;
    FTP_STATISTICS_0 * pstats;

    printf( "Invoking I_FtpQueryStatistics..." );

    err = I_FtpQueryStatistics( pszServer,
                                0,
                                (LPBYTE *)&pstats );

    if( err != NERR_Success )
    {
        printf( "ERROR %lu (%08lX)\n", err, err );
    }
    else
    {
        CHAR szLargeInt[64];

        printf( "done\n" );

        _cchNumberPadding = 13;

        if( ( pstats->TotalBytesSent.HighPart != 0 ) ||
            ( pstats->TotalBytesReceived.HighPart != 0 ) )
        {
            _cchNumberPadding = 26;
        }

        RtlLargeIntegerToChar( &pstats->TotalBytesSent,
                               10,
                               sizeof(szLargeInt),
                               szLargeInt );

        printf( "TotalBytesSent           = %s\n",
                MakeCommaString( szLargeInt )                              );

        RtlLargeIntegerToChar( &pstats->TotalBytesReceived,
                               10,
                               sizeof(szLargeInt),
                               szLargeInt );

        printf( "TotalBytesReceived       = %s\n",
                MakeCommaString( szLargeInt )                              );

        printf( "TotalFilesSent           = %s\n",
                MakeCommaNumber( pstats->TotalFilesSent )                  );

        printf( "TotalFilesReceived       = %s\n",
                MakeCommaNumber( pstats->TotalFilesReceived )              );

        printf( "CurrentAnonymousUsers    = %s\n",
                MakeCommaNumber( pstats->CurrentAnonymousUsers )           );

        printf( "CurrentNonAnonymousUsers = %s\n",
                MakeCommaNumber( pstats->CurrentNonAnonymousUsers )        );

        printf( "TotalAnonymousUsers      = %s\n",
                MakeCommaNumber( pstats->TotalAnonymousUsers )             );

        printf( "TotalNonAnonymousUsers   = %s\n",
                MakeCommaNumber( pstats->TotalNonAnonymousUsers )          );

        printf( "MaxAnonymousUsers        = %s\n",
                MakeCommaNumber( pstats->MaxAnonymousUsers )               );

        printf( "MaxNonAnonymousUsers     = %s\n",
                MakeCommaNumber( pstats->MaxNonAnonymousUsers )            );

        printf( "CurrentConnections       = %s\n",
                MakeCommaNumber( pstats->CurrentConnections )              );

        printf( "MaxConnections           = %s\n",
                MakeCommaNumber( pstats->MaxConnections )                  );

        printf( "ConnectionAttempts       = %s\n",
                MakeCommaNumber( pstats->ConnectionAttempts )              );

        printf( "LogonAttempts            = %s\n",
                MakeCommaNumber( pstats->LogonAttempts )                   );

        printf( "TimeOfLastClear          = %s\n",
                asctime( localtime( (time_t *)&pstats->TimeOfLastClear ) ) );
    }

}   // DoStats



/*****************************************************************************

    DoClear

*****************************************************************************/
VOID DoClear( WCHAR * pszServer )
{
    NET_API_STATUS   err;

    printf( "Invoking I_FtpClearStatistics..." );

    err = I_FtpClearStatistics( pszServer );

    if( err != NERR_Success )
    {
        printf( "ERROR %lu (%08lX)\n", err, err );
    }
    else
    {
        printf( "done\n" );
    }

}   // DoClear



/*****************************************************************************

    MakeCommaString

*****************************************************************************/
CHAR * MakeCommaString( CHAR * pszNumber )
{
    static CHAR   szBuffer[26];
    CHAR        * psz;
    UINT          cchNumber;
    UINT          cchNextComma;

    cchNumber  = strlen( pszNumber );
    pszNumber += cchNumber - 1;

    psz = szBuffer + _cchNumberPadding;

    *psz-- = '\0';

    cchNextComma = 3;

    while( cchNumber-- )
    {
        if( cchNextComma-- == 0 )
        {
            *psz-- = ',';
            cchNextComma = 2;
        }

        *psz-- = *pszNumber--;
    }

    while( psz >= szBuffer )
    {
        *psz-- = ' ';
    }

    return szBuffer;

}   // MakeCommaString



/*****************************************************************************

    MakeCommaNumber

*****************************************************************************/
CHAR * MakeCommaNumber( DWORD  dwNumber  )
{
    CHAR szBuffer[32];

    wsprintf( szBuffer, "%lu", dwNumber );

    return MakeCommaString( szBuffer );

}   // MakeCommaNumber

