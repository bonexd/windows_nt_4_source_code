/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    cannedsd.cxx

Abstract:

    This module contains member function definitions for the
    CANNED_SECURITY class, which is a repository for the canned
    Security Descriptors used by the utilities.

    Initializing an object of this type generates the canned
    security descriptors used by the utilities, which can
    then be gotten from the object.

    These security descriptors are all in the self-relative
    format.

    Note that this class uses the NT Api and RTL routines, rather
    than the Win32 API, since it needs to be available to AutoChk
    and AutoConvert.

Author:

    Bill McJohn (billmc) 04-March-1992

--*/

#include <pch.cxx>

#define _NTAPI_ULIB_
#define _IFSUTIL_MEMBER_

#include "ulib.hxx"
#include "ifsutil.hxx"

#include "error.hxx"
#include "ntrtl.h"
#include "nturtl.h"

#include "cannedsd.hxx"

// This generic mapping array is copied from ntos\io\ioinit.c.
//
GENERIC_MAPPING IopFileMapping = {

    STANDARD_RIGHTS_READ |
        FILE_READ_DATA | FILE_READ_ATTRIBUTES | FILE_READ_EA | SYNCHRONIZE,
    STANDARD_RIGHTS_WRITE |
        FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA | FILE_APPEND_DATA | SYNCHRONIZE,
    STANDARD_RIGHTS_EXECUTE |
        SYNCHRONIZE | FILE_READ_ATTRIBUTES | FILE_EXECUTE,
    FILE_ALL_ACCESS
};



DEFINE_EXPORTED_CONSTRUCTOR( CANNED_SECURITY, OBJECT, IFSUTIL_EXPORT );

IFSUTIL_EXPORT
CANNED_SECURITY::~CANNED_SECURITY(
    )
{
    Destroy();
}

VOID
CANNED_SECURITY::Construct(
    )
/*++

Routine Description:

    This method is the helper function for object construction.

Arguments:

    None.

Return Value:

    None.

--*/
{
    _NoAccessLength = 0;
    _NoAclLength = 0;
    _ReadLength = 0;
    _WriteLength = 0;
    _EditLength = 0;

    _NoAccessSd = NULL;
    _NoAclSd = NULL;
    _ReadSd = NULL;
    _WriteSd = NULL;
    _EditSd = NULL;
}

VOID
CANNED_SECURITY::Destroy(
    )
/*++

Routine Description:

    This method cleans up the object in preparation for destruction
    or reinitialization.

Arguments:

    None.

Return Value:

    None.

--*/
{
#if !defined( _SETUP_LOADER_ )
    _NoAccessLength = 0;
    _NoAclLength = 0;
    _ReadLength = 0;
    _WriteLength = 0;
    _EditLength = 0;

    // Since the canned security descriptors are ultimately
    // generated by RtlNewSecurityObject, they have to be
    // freed using RtlFreeHeap.
    //
    if( _NoAccessSd != NULL ) {

        RtlFreeHeap( RtlProcessHeap(), 0, _NoAccessSd );
        _NoAccessSd = NULL;
    }

    if( _NoAclSd != NULL ) {

        RtlFreeHeap( RtlProcessHeap(), 0, _NoAclSd );
        _NoAclSd = NULL;
    }

    if( _ReadSd != NULL ) {

        RtlFreeHeap( RtlProcessHeap(), 0, _ReadSd );
        _ReadSd = NULL;
    }

    if( _WriteSd != NULL ) {

        RtlFreeHeap( RtlProcessHeap(), 0, _WriteSd );
        _WriteSd = NULL;
    }

    if( _EditSd != NULL ) {

        RtlFreeHeap( RtlProcessHeap(), 0, _EditSd );
        _EditSd = NULL;
    }
#endif // _SETUP_LOADER_
}


IFSUTIL_EXPORT
BOOLEAN
CANNED_SECURITY::Initialize(
    )
/*++

Routine Description:

    This method initializes the object.  It builds the canned
    security descriptors, puts them into self-relative form,
    and squirrels them away for later use.

Arguments:

    None.

Return Value:

    TRUE upon successful completion.

--*/
{
#if defined( _SETUP_LOADER_ )

    // Canned security descriptors are not available in
    // the setup loader environment.
    //
    return FALSE;

#else

    CONST SidBufferLength = 512;
    PSID SystemSid = NULL;
    PSID AdminsSid = NULL;
    ULONG AdminsSidBufferLength, SystemSidBufferLength;
    NTSTATUS Status;
    HANDLE TokenHandle;

    Destroy();

    // Get the SID's for Admins and System.
    //
    SystemSidBufferLength = SidBufferLength;
    AdminsSidBufferLength = SidBufferLength;

    if( (SystemSid = (PSID)MALLOC( SystemSidBufferLength )) == NULL ||
        !QuerySystemSid( SystemSid, &SystemSidBufferLength ) ||
        (AdminsSid = (PSID)MALLOC( AdminsSidBufferLength )) == NULL ||
        !QueryAdminsSid( AdminsSid, &AdminsSidBufferLength ) ) {

        FREE( SystemSid );
        FREE( AdminsSid );
        Destroy();
        return FALSE;
    }

    // Get a handle to the token for the current process:
    //
    Status = NtOpenProcessToken( NtCurrentProcess(),
                                 TOKEN_QUERY,
                                 &TokenHandle );

    if( !NT_SUCCESS( Status ) ) {

        DebugPrintf( "IFSUTIL: NtOpenProcessToken failed: status = 0x%x\n", Status );
        FREE( SystemSid );
        FREE( AdminsSid );
        Destroy();
        return FALSE ;
    }

    // Generate the canned security descriptors.  All but _NoAclSd
    // have a discretionary ACL, so pass in TRUE for the DaclPresent
    // parameter for all but that exception.
    //
    _NoAccessSd = GenerateCannedSd( TRUE,
                                    0,
                                    AdminsSid,
                                    SystemSid,
                                    TokenHandle,
                                    &_NoAccessLength );

    _NoAclSd = GenerateCannedSd( FALSE,
                                 0,
                                 AdminsSid,
                                 SystemSid,
                                 TokenHandle,
                                 &_NoAclLength );

    _ReadSd = GenerateCannedSd( TRUE,
                                GENERIC_READ,
                                AdminsSid,
                                SystemSid,
                                TokenHandle,
                                &_ReadLength );

    _WriteSd = GenerateCannedSd( TRUE,
                                 GENERIC_READ | GENERIC_WRITE,
                                 AdminsSid,
                                 SystemSid,
                                 TokenHandle,
                                 &_WriteLength );

    _EditSd = GenerateCannedSd( TRUE,
                                GENERIC_ALL,
                                AdminsSid,
                                SystemSid,
                                TokenHandle,
                                &_EditLength );

    // We're done with the SID's for Admins and System, and with
    // the token for the current process.
    //
    FREE( SystemSid );
    FREE( AdminsSid );
    NtClose( TokenHandle );

    // Make sure that all the canned security descriptors were
    // successful generated.
    //
    if( _NoAccessSd == NULL ||
        _NoAclSd == NULL    ||
        _ReadSd == NULL     ||
        _WriteSd == NULL    ||
        _EditSd == NULL ) {

        Destroy();
        return FALSE;
    }

    return TRUE;

#endif // _SETUP_LOADER_
}


IFSUTIL_EXPORT
PVOID
CANNED_SECURITY::GetCannedSecurityDescriptor(
    IN  CANNED_SECURITY_TYPE   Type,
    OUT PULONG                      Length
    )
/*++

Routine Description:

    This method fetches one of the canned security descriptors.

Arguments:

    Type    --  Supplies a code indicating which security
                descriptor to get.
    Length  --  Receives the length (in bytes) of the security
                descriptor.

Return Value:

    A pointer to the canned security descriptor.  (A return value
    of NULL indicates that an error has occurred.)

--*/
{
    DebugPtrAssert( Length );

    switch( Type ) {

    case NoAccessCannedSd : *Length = _NoAccessLength;
                            return _NoAccessSd;

    case NoAclCannedSd    : *Length = _NoAclLength;
                            return _NoAclSd;

    case ReadCannedSd     : *Length = _ReadLength;
                            return _ReadSd;

    case WriteCannedSd    : *Length = _WriteLength;
                            return _WriteSd;

    case EditCannedSd     : *Length = _EditLength;
                            return _EditSd;

    default               : *Length = 0;
                            return NULL;
    }
}

#if !defined( _SETUP_LOADER_ )

BOOLEAN
QueryWorldSid(
    OUT     PSID    NewSid,
    IN OUT  PULONG  Length
    )
/*++

Routine Description:

    This functions fetches the SID for WORLD.

Arguments:

    NewSid  --  Supplies buffer in which the SID will be created.
    Length  --  Supplies the length of the buffer; receives the
                length of the actual SID.

Return Value:

    TRUE upon successful completion.

--*/
{
    SID_IDENTIFIER_AUTHORITY IdentifierAuthority = SECURITY_WORLD_SID_AUTHORITY;
    ULONG LengthNeeded;
    NTSTATUS Status;

    // WORLD is a well-known SID with one subauthority.  Make sure
    // that the buffer is big enough:
    //
    LengthNeeded = RtlLengthRequiredSid( 1 );

    if( *Length < LengthNeeded ) {

        return FALSE;
    }

    *Length = LengthNeeded;

    // Initialize the SID and fill in the subauthority:
    //
    Status = RtlInitializeSid( NewSid,
                               &IdentifierAuthority,
                               1 );

    if( !NT_SUCCESS( Status ) ) {

        DebugPrintf( "IFSUTIL: RtlInitializeSid failed--status 0x%x\n", Status );
        return FALSE;
    }

    *(RtlSubAuthoritySid( NewSid, 0 )) = SECURITY_WORLD_RID;

    return TRUE;
}

NTSTATUS
AddInheritableAccessAllowedAce(
    IN OUT PACL Acl,
    IN ULONG AceRevision,
    IN ACCESS_MASK AccessMask,
    IN PSID Sid
    )
/*++

Routine Description:

    This routine adds an ACCESS_ALLOWED ACE to an ACL.  This is
    expected to be a common form of ACL modification.  The inheritance
    flags are set so that this ACE will apply to the current item
    and will be propagated to child containers and objects.

Arguments:

    Acl - Supplies the Acl being modified

    AceRevision - Supplies the Acl/Ace revision of the ACE being added

    AccessMask - The mask of accesses to be granted to the specified SID.

    Sid - Pointer to the SID being granted access.


Return Value:

    STATUS_SUCCESS - The ACE was successfully added.
    anything else  - The ACE was not successfully added.

--*/
{
    PACCESS_ALLOWED_ACE NewAce;
    ULONG AceSize;
    NTSTATUS Status;

    AceSize = (USHORT)(sizeof(ACE_HEADER) +
                      sizeof(ACCESS_MASK) +
                      RtlLengthSid(Sid));

    NewAce = (PACCESS_ALLOWED_ACE)MALLOC( AceSize );

    if( NewAce == NULL ) {

        return STATUS_NO_MEMORY;
    }

    NewAce->Header.AceFlags = OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE;
    NewAce->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
    NewAce->Header.AceSize = (USHORT)AceSize;
    NewAce->Mask = AccessMask;
    RtlCopySid( RtlLengthSid(Sid), (PSID)(&NewAce->SidStart), Sid );

    Status = RtlAddAce( Acl, AceRevision, MAXULONG, NewAce, AceSize );

    FREE( NewAce );

    return Status;
}


BOOLEAN
GenerateWorldAcl(
    OUT PACL        AclBuffer,
    IN  ULONG       BufferLength
    )
/*++

Routine Description:

    This function creates an ACL that grants full access to all
    the world.

Arguments:

    AclBuffer       --  Supplies the buffer in which the ACL
                        will be created.
    BufferLength    --  Supplies the length of the buffer

Return Value:

    TRUE upon successful completion.

--*/
{
    CONST WorldSidBufferLength = 64;
    BYTE WorldSidBuffer[WorldSidBufferLength];

    ULONG WorldSidLength = WorldSidBufferLength;
    PSID  WorldSid = (PSID)WorldSidBuffer;

    if( !QueryWorldSid( WorldSid, &WorldSidLength ) ) {

        return FALSE;
    }

    ACL_SIZE_INFORMATION AclSizeInfo;
    PACL NewAcl;
    NTSTATUS Status;

    // This is rather complicated, since we want to use the minimum
    // space for this ACL, but have to give it the buffer size in
    // advance.  To deal with this problem, we build the ACL up
    // twice.  The first time, we initialize it with the entire
    // buffer.  Once it's complete, we can ask it how big it really
    // is, and rebuild it with the correct size.
    //
    NewAcl = (PACL)AclBuffer;
    Status = RtlCreateAcl( NewAcl, BufferLength, ACL_REVISION );

    if( !NT_SUCCESS( Status ) ) {

        DebugPrintf( "IFSUTIL: RtlCreateAcl failed--status 0x%x.\n", Status );
        return FALSE;
    }

    // This ACL always has one ACE, which grants the world full
    // access:
    //
    Status = AddInheritableAccessAllowedAce( NewAcl,
                                             ACL_REVISION,
                                             GENERIC_ALL,
                                             WorldSid );

    if( !NT_SUCCESS( Status ) ) {

        DebugPrintf( "IFSUTIL: RtlAddAccessAllowedAce failed--status 0x%x.\n", Status );
        return FALSE;
    }


    // Now determine the size of this ACL:
    //
    RtlQueryInformationAcl( NewAcl,
                            &AclSizeInfo,
                            sizeof( ACL_SIZE_INFORMATION ),
                            AclSizeInformation );

    // Now reinitialize and rebuild the ACL with the correct size:
    //
    Status = RtlCreateAcl( NewAcl, AclSizeInfo.AclBytesInUse, ACL_REVISION );

    if( !NT_SUCCESS( Status ) ) {

        DebugPrintf( "IFSUTIL: RtlCreateAcl failed--status 0x%x.\n", Status );
        return FALSE;
    }

    Status = AddInheritableAccessAllowedAce( NewAcl,
                                             ACL_REVISION,
                                             GENERIC_ALL,
                                             WorldSid );

    if( !NT_SUCCESS( Status ) ) {

        DebugPrintf( "IFSUTIL: RtlAddAccessAllowedAce failed--status 0x%x.\n", Status );
        return FALSE;
    }


    return TRUE;
}


BOOLEAN
CANNED_SECURITY::QuerySystemSid(
    OUT    PSID     NewSid,
    IN OUT PULONG   Length
    )
/*++

Routine Description:

    This method fetches the SID for the SYSTEM account.

Arguments:

    NewSid  --  Supplies the buffer in which the SID will be created.
    Length  --  Supplies the length of the buffer; receives the length
                of the actual SID.

Return Value:

    TRUE upon successful completion.

--*/
{
    SID_IDENTIFIER_AUTHORITY IdentifierAuthority = SECURITY_NT_AUTHORITY;
    ULONG LengthNeeded;
    NTSTATUS Status;

    // System is a well-known SID with one subauthority.  Make sure
    // that the buffer is big enough:
    //
    LengthNeeded = RtlLengthRequiredSid( 1 );

    if( *Length < LengthNeeded ) {

        return FALSE;
    }

    *Length = LengthNeeded;

    // Initialize the SID and fill in the subauthority:
    //
    Status = RtlInitializeSid( NewSid,
                               &IdentifierAuthority,
                               1 );

    if( !NT_SUCCESS( Status ) ) {

        DebugPrintf( "IFSUTIL: RtlInitializeSid failed--status 0x%x\n", Status );
        return FALSE;
    }

    *(RtlSubAuthoritySid( NewSid, 0 )) = SECURITY_LOCAL_SYSTEM_RID;

    return TRUE;
}


BOOLEAN
CANNED_SECURITY::QueryAdminsSid(
    OUT    PSID     NewSid,
    IN OUT PULONG   Length
    )
/*++

Routine Description:

    This method fetches the SID for the WORKSTATION ADMINISTRATORS
    alias.

Arguments:

    NewSid  --  Supplies the buffer in which the SID will be created.
    Length  --  Supplies the length of the buffer; receives the length
                of the actual SID.

Return Value:

    TRUE upon successful completion.

--*/
{
    SID_IDENTIFIER_AUTHORITY IdentifierAuthority = SECURITY_NT_AUTHORITY;
    ULONG LengthNeeded;
    NTSTATUS Status;

    // Admins is a well-known SID with two subauthorities.  Make sure
    // that the buffer is big enough:
    //
    LengthNeeded = RtlLengthRequiredSid( 2 );

    if( *Length < LengthNeeded ) {

        return FALSE;
    }

    *Length = LengthNeeded;

    // Initialize the SID and fill in the two subauthorities:
    //
    Status = RtlInitializeSid( NewSid,
                               &IdentifierAuthority,
                               2 );

    if( !NT_SUCCESS( Status ) ) {

        DebugPrintf( "IFSUTIL: RtlInitializeSid failed--status 0x%x\n", Status );
        return FALSE;
    }

    *(RtlSubAuthoritySid( NewSid, 0 )) = SECURITY_BUILTIN_DOMAIN_RID;
    *(RtlSubAuthoritySid( NewSid, 1 )) = DOMAIN_ALIAS_RID_ADMINS;

    return TRUE;
}



PVOID
CANNED_SECURITY::GenerateCannedSd(
    IN  BOOLEAN     DaclPresent,
    IN  ACCESS_MASK GrantedAccess,
    IN  PSID        AdminsSid,
    IN  PSID        SystemSid,
    IN  HANDLE      TokenHandle,
    OUT PULONG      Length
    )
/*++

Routine Description:

    This method generates a self-relative Security Descriptor
    which grants the specified access to System and Administrators.

Arguments:

    DaclPresent     --  Supplies a flag that indicates whether this
                        Security Descriptor will have a discretionary
                        ACL.  If this flag is FALSE, the GrantedAccess
                        parameter is ignored.
    GrantedAccess   --  Supplies the access that System and Administrators
                        will have to the file protected by this Security
                        Descriptor.  A value of zero indicates that no
                        aces are to be created.
    AdminsSid       --  Supplies the SID for Administrators.
    SystemSid       --  Supplies the SID for System.
    Length          --  Receives the length of the Security Descriptor.

Return Value:

    A pointer to the generated Security Descriptor.  NULL indicates
    failure.

--*/
{
    CONST CannedAclBufferLength = 2048;
    STATIC BYTE CannedAclBuffer[CannedAclBufferLength];

    SECURITY_DESCRIPTOR AbsoluteSd;
    PSECURITY_DESCRIPTOR SelfRelativeSd;
    NTSTATUS Status;

    // Create an absolute Security Descriptor.
    //
    Status = RtlCreateSecurityDescriptor( &AbsoluteSd,
                                          SECURITY_DESCRIPTOR_REVISION );

    if( !NT_SUCCESS( Status ) ) {

        DebugPrintf( "RtlCreateSecurityDescriptor failed--status 0x%x.\n", Status );
        return NULL;
    }

    // The owner and group is ADMINS.
    //
    Status = RtlSetOwnerSecurityDescriptor( &AbsoluteSd,
                                            AdminsSid,
                                            TRUE );

    if( !NT_SUCCESS( Status ) ) {

        DebugPrintf( "IFSUTIL: RtlSetOwnerSecurityDescriptor failed--status 0x%x\n", Status );
        return NULL;
    }

    Status = RtlSetGroupSecurityDescriptor( &AbsoluteSd,
                                            AdminsSid,
                                            TRUE );

    if( !NT_SUCCESS( Status ) ) {

        DebugPrintf( "IFSUTIL: RtlSetGroupSecurityDescriptor failed--status 0x%x\n", Status );
        return NULL;
    }

    if( DaclPresent ) {

        // The client wants to put a discretionary ACL on this
        // security descriptor.
        //
        if( !GenerateCannedAcl( (PACL)CannedAclBuffer,
                                CannedAclBufferLength,
                                GrantedAccess,
                                AdminsSid,
                                SystemSid ) ) {

            return NULL;
        }

        // Attach the new ACL to the Security Descriptor.  Pass in
        // TRUE for DaclPresent (since that's what we're setting)
        // and for DaclDefaulted (since these canned Security
        // Descriptors are defaults).
        //
        Status = RtlSetDaclSecurityDescriptor( &AbsoluteSd,
                                               TRUE,
                                               (PACL)CannedAclBuffer,
                                               TRUE );

        if( !NT_SUCCESS( Status ) ) {

            DebugPrintf( "RtlSetDaclSecurityDescriptor failed--status 0x%x.\n", Status );
            return NULL;
        }

    } else {

        // The client has not specified a discretionary ACL, so
        // we'll create an ACL that grants full access to all
        // the world.
        //
        if( !GenerateWorldAcl( (PACL)CannedAclBuffer,
                               CannedAclBufferLength ) ){

            return NULL;
        }


        // Attach the new ACL to the Security Descriptor.  Pass in
        // TRUE for DaclPresent (since that's what we're setting)
        // and for DaclDefaulted (since these canned Security
        // Descriptors are defaults).
        //
        Status = RtlSetDaclSecurityDescriptor( &AbsoluteSd,
                                               TRUE,
                                               (PACL)CannedAclBuffer,
                                               TRUE );

        if( !NT_SUCCESS( Status ) ) {

            DebugPrintf( "RtlSetDaclSecurityDescriptor failed--status 0x%x.\n", Status );
            return NULL;
        }
    }

    // Now call RtlNewSecurityObject to massage this security
    // descriptor into a self-relative security descriptor
    // that canned be slammed onto files.
    //
    Status = RtlNewSecurityObject( NULL,
                                   &AbsoluteSd,
                                   &SelfRelativeSd,
                                   FALSE,
                                   TokenHandle,
                                   &IopFileMapping );

    if( !NT_SUCCESS( Status ) ) {

        DebugPrintf( "IFSUTIL:  RtlNewSecurityObject failed--status = 0x%x\n", Status );
        return NULL;
    }

    // Make sure the returned security descriptor is valid:
    //
    if( !RtlValidSecurityDescriptor( SelfRelativeSd ) ) {

        DebugPrint( "IFSUTIL: RtlNewSecurityObject did not return a valid security descriptor.\n" );

        RtlFreeHeap( RtlProcessHeap(), 0, SelfRelativeSd );
        return NULL;
    }

    *Length = RtlLengthSecurityDescriptor( SelfRelativeSd );
    return SelfRelativeSd;
}


BOOLEAN
CANNED_SECURITY::GenerateCannedAcl(
    IN  PACL        AclBuffer,
    IN  ULONG       BufferLength,
    IN  ACCESS_MASK GrantedAccess,
    IN  PSID        AdminsSid,
    IN  PSID        SystemSid
    )
/*++

Routine Description:

    This method builds up an ACL in the supplied buffer.

Arguments:

    AclBuffer       --  Supplies the buffer in which to build the ACL.
    BufferLength    --  Supplies the length of the buffer (in bytes).
    GrantedAccess   --  Supplies the access the ACL will grant to
                        SYSTEM and ADMINS.
    AdminsSid       --  Supplies the SID for administrators.
    SystemSid       --  Supplies the SID for SYSTEM.

--*/
{
    ACL_SIZE_INFORMATION AclSizeInfo;
    PACL NewAcl;
    NTSTATUS Status;

    // This is rather complicated, since we want to use the minimum
    // space for this ACL, but have to give it the buffer size in
    // advance.  To deal with this problem, we build the ACL up
    // twice.  The first time, we initialize it with the entire
    // buffer.  Once it's complete, we can ask it how big it really
    // is, and rebuild it with the correct size.
    //
    NewAcl = (PACL)AclBuffer;
    Status = RtlCreateAcl( NewAcl, BufferLength, ACL_REVISION );

    if( !NT_SUCCESS( Status ) ) {

        DebugPrintf( "IFSUTIL: RtlCreateAcl failed--status 0x%x.\n", Status );
        return FALSE;
    }

    if( GrantedAccess != 0 ) {

        // The client wants an ACL that grants certain access
        // rights to System and Administrators.
        //
        Status = RtlAddAccessAllowedAce( NewAcl,
                                         ACL_REVISION,
                                         GrantedAccess,
                                         SystemSid );

        if( !NT_SUCCESS( Status ) ) {

            DebugPrintf( "IFSUTIL: RtlAddAccessAllowedAce failed--status 0x%x.\n", Status );
            return FALSE;
        }

        Status = RtlAddAccessAllowedAce( NewAcl,
                                         ACL_REVISION,
                                         GrantedAccess,
                                         AdminsSid );

        if( !NT_SUCCESS( Status ) ) {

            DebugPrintf( "IFSUTIL: RtlAddAccessAllowedAce failed--status 0x%x.\n", Status );
            return FALSE;
        }
    }

    // Now determine the size of this ACL:
    //
    RtlQueryInformationAcl( NewAcl,
                            &AclSizeInfo,
                            sizeof( ACL_SIZE_INFORMATION ),
                            AclSizeInformation );

    // Now reinitialize and rebuild the ACL with the correct size:
    //
    Status = RtlCreateAcl( NewAcl, AclSizeInfo.AclBytesInUse, ACL_REVISION );

    if( !NT_SUCCESS( Status ) ) {

        DebugPrintf( "IFSUTIL: RtlCreateAcl failed--status 0x%x.\n", Status );
        return FALSE;
    }

    if( GrantedAccess != 0 ) {

        // The client wants an ACL that grants certain access
        // rights to System and Administrators.
        //
        Status = RtlAddAccessAllowedAce( NewAcl,
                                         ACL_REVISION,
                                         GrantedAccess,
                                         SystemSid );

        if( !NT_SUCCESS( Status ) ) {

            DebugPrintf( "IFSUTIL: RtlAddAccessAllowedAce failed--status 0x%x.\n", Status );
            return FALSE;
        }

        Status = RtlAddAccessAllowedAce( NewAcl,
                                         ACL_REVISION,
                                         GrantedAccess,
                                         AdminsSid );

        if( !NT_SUCCESS( Status ) ) {

            DebugPrintf( "IFSUTIL: RtlAddAccessAllowedAce failed--status 0x%x.\n", Status );
            return FALSE;
        }
    }

    return TRUE;
}


#endif // _SETUP_LOADER_
