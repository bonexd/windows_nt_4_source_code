/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    groupman.c

Abstract:

    Contains code for the Group List Database manager.  This includes
    all the linked list routines.  This file contains the following
    functions:
        ScGetOrderGroupList
        ScGetStandaloneGroupList
        ScGetUnresolvedDependList
        ScGetNamedGroupRecord

        ScCreateOrderGroupEntry
        ScAllocateGroupEntry
        ScCreateGroupMembership
        ScDeleteGroupMembership
        ScCreateRegistryGroupPointer
        ScDeleteRegistryGroupPointer
        ScCreateStandaloneGroup
        ScDeleteStandaloneGroup

        ScGenerateDependencies
        ScSetDependencyPointers
        ScResolveDependencyToService
        ScCreateDependencies
        ScCreateUnresolvedDepend
        ScDeleteUnresolvedDepend
        ScCreateDependRecord
        ScDeleteStartDependencies
        ScDeleteStopDependencies
        ScSetServiceDependList
        ScChangeGroupOrder
        ScFindStandaloneGroup
        ScFindOrderGroup
        ScSetStandaloneGroupRefcount
        ScGetUniqueTag
        ScCompareVector

        ScGetDependencySize
        ScGetDependencyString

        ScDumpGroups
        ScDumpServiceDependencies

Author:

    Dan Lafferty (danl)     04-Feb-1992

Environment:

    User Mode -Win32

Revision History:

    22-Oct-1993     danl
        Created by splitting these functions out of dataman.c because it was
        getting too large.

--*/

//
// INCLUDES
//

#include <nt.h>         // for ntrtl.h
#include <ntrtl.h>      // DbgPrint prototype
#include <rpc.h>        // DataTypes and runtime APIs
#include <nturtl.h>     // needed for windows.h when I have nt.h
#include <windows.h>    // LocalAlloc
#include <stdlib.h>      // wide character c runtimes.

#include <tstr.h>       // Unicode string macros

#include <winsvc.h>     // public Service Controller Interface.

#include <scdebug.h>    // SC_LOG(), SC_ASSERT().
#include <ntrpcp.h>     // MIDL_user_allocate
#include <control.h>    // SendControl
#include "dataman.h"    // dataman structures
#include "scopen.h"     // SERVICE_SIGNATURE
#include "scconfig.h"   // ScGenerateServiceDB,ScInitSecurityProcess
#include "svcctrl.h"    // ScRemoveServiceBits
#include "scsec.h"      // ScCreateScServiceObject
#include "account.h"    // ScRemoveAccount
#include <sclib.h>      // ScImagePathsMatch().
#include "bootcfg.h"    // ScDeleteRegTree().
#include <strarray.h>   // ScWStrArraySize


//
// Defines
//

// Names of specially treated groups
#define SC_GROUPNAME_TDI        L"TDI"
#define SC_GROUPNAME_PNP_TDI    L"PNP_TDI"
#define SC_GROUPNAME_NDIS       L"NDIS"
#define SC_GROUPNAME_NDISWAN    L"NDISWAN"

// A value that will not match any real pointer to a load order group
#define SC_INVALID_GROUP    ((LPLOAD_ORDER_GROUP) 0xFFFFFFFF)


//
// External Globals
//

    //
    // TDI GROUP SPECIAL:  The groups named TDI and PNP_TDI are treated
    // specially during dependency handling.  This is done by remembering a
    // pointer to each of those groups, if it occurs in the group order list,
    // and checking against the remembered pointers during dependency
    // handling.
    // NDIS GROUP SPECIAL: The groups named NDIS and NDISWAN are treated
    // specially during driver loading.  This is done in the same way as above.
    //

    LPLOAD_ORDER_GROUP  ScGlobalTDIGroup     = SC_INVALID_GROUP;
    LPLOAD_ORDER_GROUP  ScGlobalPNP_TDIGroup = SC_INVALID_GROUP;
    LPLOAD_ORDER_GROUP  ScGlobalNDISGroup    = SC_INVALID_GROUP;
    LPLOAD_ORDER_GROUP  ScGlobalNDISWANGroup = SC_INVALID_GROUP;

//
//  Static Globals
//

    //
    // These are the linked list heads for each of the databases
    // that are maintained.
    //

    LOAD_ORDER_GROUP  OrderGroupList;       // empty header for doubly linked
    LOAD_ORDER_GROUP  StandaloneGroupList;  // empty header for doubly linked

    UNRESOLVED_DEPEND UnresolvedDependList; // empty header for doubly linked

//
// Local Function Prototypes
//

DWORD
ScAllocateOrderGroupEntry(
    OUT LPLOAD_ORDER_GROUP *NewGroup,
    IN  LPWSTR GroupName
    );

DWORD
ScCreateStandaloneGroup(
    IN  LPWSTR GroupName,
    OUT LPLOAD_ORDER_GROUP *GroupPointer
    );

VOID
ScDeleteStandaloneGroup(
    IN LPLOAD_ORDER_GROUP Group
    );

VOID
ScRememberSpecialGroup(
    IN  LPLOAD_ORDER_GROUP Group
    );

VOID
ScForgetSpecialGroup(
    IN  LPLOAD_ORDER_GROUP Group
    );

DWORD
ScCreateUnresolvedDepend(
   IN  LPWSTR Name,
   OUT LPUNRESOLVED_DEPEND *Unresolved
   );

VOID
ScDeleteUnresolvedDepend(
    IN OUT LPUNRESOLVED_DEPEND *Unresolved
    );

DWORD
ScSetServiceDependList(
    LPDEPEND_RECORD Start,
    LPSERVICE_RECORD ServiceRecord,
    PVOID DependOnRecord,
    DEPEND_TYPE DependOnType
    );

LPLOAD_ORDER_GROUP
ScFindStandaloneGroup(
    IN LPWSTR GroupName
    );

LPLOAD_ORDER_GROUP
ScFindOrderGroup(
    IN LPLOAD_ORDER_GROUP Group,
    IN LPWSTR GroupName
    );

VOID
ScSetStandaloneGroupRefcount(
    IN LPLOAD_ORDER_GROUP StandaloneGroup
    );

VOID
ScCompareVector(
    IN     LPDWORD TagArray,
    IN     DWORD TagArrayLength,
    IN OUT LPDWORD ReturnTagPtr
    );



//****************************************************************************/
// Miscellaneous Short Functions
//****************************************************************************/
LPLOAD_ORDER_GROUP
ScGetOrderGroupList(
    VOID
    )
{
    return OrderGroupList.Next;
}

LPLOAD_ORDER_GROUP
ScGetStandaloneGroupList(
    VOID
    )
{
    return StandaloneGroupList.Next;
}

LPUNRESOLVED_DEPEND
ScGetUnresolvedDependList(
    VOID
    )
{
    return UnresolvedDependList.Next;
}

VOID
ScInitGroupDatabase(VOID)

/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    OrderGroupList.Next = NULL;
    OrderGroupList.Prev = NULL;

    StandaloneGroupList.Next = NULL;
    StandaloneGroupList.Prev = NULL;

    UnresolvedDependList.Next = NULL;
    UnresolvedDependList.Prev = NULL;

}

VOID
ScEndGroupDatabase(VOID)

/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    LPLOAD_ORDER_GROUP Group;
    LPLOAD_ORDER_GROUP Grp;

    Group = OrderGroupList.Next;

    while (Group != NULL) {

        Grp = Group;
        Group = Group->Next;

        REMOVE_FROM_LIST(Grp);
        (VOID) LocalFree((HLOCAL)Grp);
    }


}


DWORD
ScCreateOrderGroupEntry(
    IN  LPWSTR GroupName
    )
/*++

Routine Description:

    This function adds a group entry into the end of the load order group
    list.

Arguments:

    GroupName - Supplies the name of the load group.

Return Value:

    NO_ERROR - The operation was successful.

    ERROR_NOT_ENOUGH_MEMORY - The call to allocate memory for a new
        group entry failed.

Note:

    The GroupListLock must be held exclusively prior to calling this function.

--*/
{
    DWORD status;
    LPLOAD_ORDER_GROUP NewGroup;
    LPLOAD_ORDER_GROUP GroupListPointer;


    if ((status = ScAllocateOrderGroupEntry(
                      &NewGroup,
                      GroupName
                      )) != NO_ERROR) {
        return status;
    }

    GroupListPointer = &OrderGroupList;

    //
    // Add the group entry to the group list at the end.
    //
    ADD_TO_LIST(GroupListPointer, NewGroup);

    SC_LOG(CONFIG, "ScCreateOrderGroupEntry: Added %ws to GroupList\n", GroupName);

    return NO_ERROR;
}


DWORD
ScAllocateOrderGroupEntry(
    OUT LPLOAD_ORDER_GROUP *NewGroup,
    IN  LPWSTR GroupName
    )
{

    //
    // Allocate memory for the new group.
    //
    *NewGroup = (LPLOAD_ORDER_GROUP)LocalAlloc(
                    LMEM_ZEROINIT,
                    WCSSIZE(GroupName) + sizeof(LOAD_ORDER_GROUP)
                    );

    if (*NewGroup == NULL) {
        SC_LOG(ERROR,"ScAllocateOrderGroupEntry: LocalAlloc failure %ld\n",
               GetLastError());
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    //
    // Save away the GroupName
    //
    (*NewGroup)->GroupName = (LPWSTR) ((LPBYTE) (*NewGroup) + sizeof(LOAD_ORDER_GROUP));
    wcscpy((*NewGroup)->GroupName, GroupName);

    ScRememberSpecialGroup(*NewGroup);

    //
    // Set the RefCount field to 0xffffffff so that we can differentiate an
    // order group from a standalone group.  This field actually indicates
    // the number of members in a group and dependency references to it if
    // the group is standalone so that we can delete the standalone group
    // when it goes to 0.
    //
    (*NewGroup)->RefCount = MAXULONG;

    return NO_ERROR;
}


DWORD
ScCreateGroupMembership(
    OUT PSERVICE_RECORD ServiceRecord,
    IN  LPWSTR Group OPTIONAL
    )
/*++

Routine Description:

    This function assigns the load order group membership information
    of the service to its specified service record.  If the service
    belongs to a group in OrderGroupList, a pointer to the group in the load
    order group list is saved.  If the service belongs to a group which
    is not in the load order group list, the name of the group is saved
    in the service record in case the group gets added to the load order
    group list later.  If Group is not specified, no group membership
    information is saved.

Arguments:

    ServiceRecord - Receives the group membership information in this service
        record.

    Group - Supplies the string which contains the name of the group.  This
        is the raw string read from the registry which may contain blank,
        tab or newline characters which we should ignore.

Return Value:

    NO_ERROR - The operation was successful.

    ERROR_NOT_ENOUGH_MEMORY - The call to allocate memory for a group name
        failed.

Note:

    This routine assumes that the database lock is already held.  It is
    called by ScAddConfigInfoServiceRecord.

    It also assumes that the caller has exclusive access to the group
    list lock.

--*/
{
    DWORD status;
    LPWSTR GroupPtr = Group;
    LPWSTR GroupName;
    PLOAD_ORDER_GROUP GroupEntry = ScGetOrderGroupList();


    //
    // Extract the group name from the string read in from the registry.
    //
    if ((! ARGUMENT_PRESENT(GroupPtr)) || (! ScGetToken(&GroupPtr, &GroupName))) {
        ServiceRecord->MemberOfGroup = (PLOAD_ORDER_GROUP) NULL;
        return NO_ERROR;
    }

    //
    // Search for matching group name in load order list
    //
    while (GroupEntry != NULL) {
        if (_wcsicmp(GroupEntry->GroupName, GroupName) == 0) {
            ServiceRecord->MemberOfGroup = GroupEntry;
            return NO_ERROR;
        }
        GroupEntry = GroupEntry->Next;
    }

    //
    // Group name not NULL, and not found in load order group list.
    // Group is a standalone group.
    //
    status = ScCreateStandaloneGroup(
                 GroupName,
                 &(ServiceRecord->MemberOfGroup)
                 );

    if (status != NO_ERROR) {
        return status;
    }

    return NO_ERROR;
}


VOID
ScDeleteGroupMembership(
    IN OUT PSERVICE_RECORD ServiceRecord
    )
/*++

Routine Description:

    This function deletes any memory allocated for group membership
    association.

Arguments:

    ServiceRecord - Supplies the group membership information in this
        service record.

Return Value:

    None.

Note:

    This routine assumes that the database lock is already held.  It is
    called by ScAddConfigInfoServiceRecord and ScDecrementUseCountAndDelete.

    It also assumes that the group list lock is held exclusively.

--*/
{

    if (ServiceRecord->MemberOfGroup != NULL &&
        ServiceRecord->MemberOfGroup->RefCount != MAXULONG) {
        ScDeleteStandaloneGroup(ServiceRecord->MemberOfGroup);
    }

    ServiceRecord->MemberOfGroup = NULL;
}


DWORD
ScCreateRegistryGroupPointer(
    OUT PSERVICE_RECORD ServiceRecord,
    IN  LPWSTR Group OPTIONAL
    )
/*++

Routine Description:

    This function assigns the load order group RegistryGroup
    information in the service record to match the load order group
    stored in the registry, which is not the same as MemberOfGroup
    information if the load order group of the service is changed
    while the service is running.  However, we need to know what the
    resultant load order group of the service is when it stops so
    that when we can guarantee uniqueness of a tag based on all
    members the group.

    This function does exactly the same thing as the
    ScCreateGroupMembership function but alters the RegistryGroup
    pointer instead of the MemberOfGroup pointer in the service
    record.

Arguments:

    ServiceRecord - Receives the group membership information in this service
        record.

    Group - Supplies the string which contains the name of the group.  This
        is the raw string read from the registry which may contain blank,
        tab or newline characters which we should ignore.

Return Value:

    NO_ERROR - The operation was successful.

    ERROR_NOT_ENOUGH_MEMORY - The call to allocate memory for a group name
        failed.

Note:

    This routine assumes that the database lock is already held.  It is
    called by ScAddConfigInfoServiceRecord.

    It also assumes that the caller has exclusive access to the group
    list lock.

--*/
{
    DWORD status;
    LPWSTR GroupPtr = Group;
    LPWSTR GroupName;
    PLOAD_ORDER_GROUP GroupEntry = ScGetOrderGroupList();


    //
    // Extract the group name from the string read in from the registry.
    //
    if ((! ARGUMENT_PRESENT(GroupPtr)) || (! ScGetToken(&GroupPtr, &GroupName))) {
        ServiceRecord->RegistryGroup = (PLOAD_ORDER_GROUP) NULL;
        return NO_ERROR;
    }

    //
    // Search for matching group name in load order list
    //
    while (GroupEntry != NULL) {
        if (_wcsicmp(GroupEntry->GroupName, GroupName) == 0) {
            ServiceRecord->RegistryGroup = GroupEntry;
            return NO_ERROR;
        }
        GroupEntry = GroupEntry->Next;
    }

    //
    // Group name not NULL, and not found in load order group list.
    // Group is a standalone group.
    //
    status = ScCreateStandaloneGroup(
                 GroupName,
                 &(ServiceRecord->RegistryGroup)
                 );

    if (status != NO_ERROR) {
        return status;
    }

    return NO_ERROR;
}


VOID
ScDeleteRegistryGroupPointer(
    IN OUT PSERVICE_RECORD ServiceRecord
    )
/*++

Routine Description:

    This function deletes any memory allocated for registry group
    association.

Arguments:

    ServiceRecord - Supplies the registry group information in this
        service record.

Return Value:

    None.

Note:

    This routine assumes that the database lock is already held.  It is
    called by ScAddConfigInfoServiceRecord and ScDecrementUseCountAndDelete.

    It also assumes that the group list lock is held exclusively.

--*/
{

    if (ServiceRecord->RegistryGroup != NULL &&
        ServiceRecord->RegistryGroup->RefCount != MAXULONG) {
        ScDeleteStandaloneGroup(ServiceRecord->RegistryGroup);
    }

    ServiceRecord->RegistryGroup = NULL;
}


DWORD
ScCreateStandaloneGroup(
    IN LPWSTR GroupName,
    OUT LPLOAD_ORDER_GROUP *GroupPointer
    )
/*++

Routine Description:

    This function looks for a matching standalone group entry in the
    standalone group list.  If a match is found, the reference count is
    incremented and the pointer to the matching entry is returned.

    If no matching entry is found, this function creates a new standalone
    group entry, insert it into the end of the standalone group list, and
    return a pointer to the new entry.


Arguments:

    Name - Supplies the name of the group which is not in the
        ServiceOrderList.

    GroupPointer - Receives a pointer to the unresolved entry.

Return Value:

    NO_ERROR - The operation was successful.

    ERROR_NOT_ENOUGH_MEMORY - Allocation of memory failed.

Note:

    This routine assumes that the caller has exclusively acquired the
    group list lock.  It is called by ScCreateGroupMembership.

--*/
{

    LPLOAD_ORDER_GROUP Group = ScGetStandaloneGroupList();

    BOOL Found = FALSE;


    //
    // Search the existing standalone group list for the matching
    // standalone group entry.
    //
    while (Group != NULL) {

        if (_wcsicmp(Group->GroupName, GroupName) == 0) {
            Found = TRUE;
            break;
        }

        Group = Group->Next;
    }

    if (Found) {

        Group->RefCount++;

        SC_LOG2(DEPEND_DUMP,
            "Found existing group entry for " FORMAT_LPWSTR
            ", just increment refcount to %lu\n", Group->GroupName,
            Group->RefCount);

        *GroupPointer = Group;

        return NO_ERROR;
    }

    //
    // Not found.  Allocate a new group entry.
    //
    if ((*GroupPointer = (LPLOAD_ORDER_GROUP)LocalAlloc(
                             LMEM_ZEROINIT,
                             sizeof(LOAD_ORDER_GROUP) + WCSSIZE(GroupName)
                             )) == NULL) {
        SC_LOG(ERROR,"ScCreateStandaloneGroup: LocalAlloc failure %lu\n",
               GetLastError());
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    (*GroupPointer)->GroupName = (LPWSTR) ((DWORD) *GroupPointer +
                                    sizeof(LOAD_ORDER_GROUP));
    wcscpy((*GroupPointer)->GroupName, GroupName);

    (*GroupPointer)->RefCount = 1;

    SC_LOG1(DEPEND_DUMP, "Created new standalone group entry "
            FORMAT_LPWSTR "\n", (*GroupPointer)->GroupName);

    ScRememberSpecialGroup(*GroupPointer);

    Group = &StandaloneGroupList;

    //
    // Add the new group entry to the standalone group list at the end.
    //
    ADD_TO_LIST(Group, *GroupPointer);

    return NO_ERROR;
}


VOID
ScDeleteStandaloneGroup(
    IN LPLOAD_ORDER_GROUP Group
    )
{
    if (Group->RefCount) {
        Group->RefCount--;

        SC_LOG1(DEPEND, "DeleteStandaloneGroup: Subtracted RefCount is "
                FORMAT_DWORD "\n", Group->RefCount);
    }
    else {
        SC_LOG0(ERROR, "ScDeleteStandaloneGroup: Before delete, refcount is 0!\n");
        SC_ASSERT(FALSE);
    }

    if (Group->RefCount == 0) {

        SC_LOG1(DEPEND, "Deleting standalone group " FORMAT_LPWSTR "\n",
                Group->GroupName);

        REMOVE_FROM_LIST(Group);

        ScForgetSpecialGroup(Group);

        (void) LocalFree((HLOCAL)Group);
    }
}



VOID
ScRememberSpecialGroup(
    IN  LPLOAD_ORDER_GROUP Group
    )

/*++

Routine Description:

    Compares the group name against a set of known group names to see if it
    is a group that requires special handling, and if so, saves the pointer
    to the group in a global variable.


Arguments:


Return Value:


--*/
{
    // CODEWORK:  If the number of special groups keeps growing, do this
    // in a table-driven way!

    if (_wcsicmp(Group->GroupName, SC_GROUPNAME_TDI) == 0)
    {
        if (ScGlobalTDIGroup != SC_INVALID_GROUP)
        {
            SC_LOG0(ERROR, "Warning: TDI group occurs more than once in load order group list\n");
        }
        ScGlobalTDIGroup = Group;
    }
    else if (_wcsicmp(Group->GroupName, SC_GROUPNAME_PNP_TDI) == 0)
    {
        if (ScGlobalPNP_TDIGroup != SC_INVALID_GROUP)
        {
            SC_LOG0(ERROR, "Warning: PNP_TDI group occurs more than once in load order group list\n");
        }
        ScGlobalPNP_TDIGroup = Group;
    }
    else if (_wcsicmp(Group->GroupName, SC_GROUPNAME_NDIS) == 0)
    {
        if (ScGlobalNDISGroup != SC_INVALID_GROUP)
        {
            SC_LOG0(ERROR, "Warning: NDIS group occurs more than once in load order group list\n");
        }
        ScGlobalNDISGroup = Group;
    }
    else if (_wcsicmp(Group->GroupName, SC_GROUPNAME_NDISWAN) == 0)
    {
        if (ScGlobalNDISWANGroup != SC_INVALID_GROUP)
        {
            SC_LOG0(ERROR, "Warning: NDISWAN group occurs more than once in load order group list\n");
        }
        ScGlobalNDISWANGroup = Group;
    }
}


VOID
ScForgetSpecialGroup(
    IN  LPLOAD_ORDER_GROUP Group
    )

/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    if (Group == ScGlobalTDIGroup)
    {
        ScGlobalTDIGroup = SC_INVALID_GROUP;
    }
    else if (Group == ScGlobalPNP_TDIGroup)
    {
        ScGlobalPNP_TDIGroup = SC_INVALID_GROUP;
    }
    else if (Group == ScGlobalNDISGroup)
    {
        ScGlobalNDISGroup = SC_INVALID_GROUP;
    }
    else if (Group == ScGlobalNDISWANGroup)
    {
        ScGlobalNDISWANGroup = SC_INVALID_GROUP;
    }
}



VOID
ScGenerateDependencies(
    VOID
    )

/*++

Routine Description:


Arguments:


Return Value:


Note:

    The GroupListLock must be held exclusively prior to calling this routine.


--*/
{

    LPSERVICE_RECORD Service = ScGetServiceDatabase();


    while (Service != NULL) {

        (void) ScSetDependencyPointers(Service);
        Service = Service->Next;
    }

}


DWORD
ScSetDependencyPointers(
    IN LPSERVICE_RECORD Service
    )

/*++

Routine Description:


Arguments:


Return Value:


Note:

    The GroupListLock must be held exclusively prior to calling this routine.


--*/
{

    DWORD status;


    if (Service->Dependencies != NULL) {

        status = ScCreateDependencies(
                     Service,
                     Service->Dependencies
                     );

        if (status == NO_ERROR) {
            (void) LocalFree((HLOCAL)Service->Dependencies);
            Service->Dependencies = NULL;
        }

        return status;
    }

    return NO_ERROR;
}


DWORD
ScResolveDependencyToService(
    LPSERVICE_RECORD DependOnService
    )
/*++

Routine Description:

    This function resolves all dependencies to the service we are
    currently installing via CreateService.  The start depend entry
    of these services will point to the service record of the service
    we are installing instead of the unresolved depend record.  A
    stop depend entry is created for the service we are installing to
    point back to every service that depends on it.

Arguments:

    DependOnService - Supplies a pointer to the service we are installing
        via CreateService which other services may depend on.

Return Value:

    NO_ERROR - The operation was successful.

    ERROR_NOT_ENOUGH_MEMORY - Fail to allocate memory for required data
        structures.

Note:

    This routine assumes that the caller has exclusively acquired the
    database lock.  It is called by RCreateServiceW.

--*/
{
    DWORD status;
    LPUNRESOLVED_DEPEND UnresolvedEntry = ScGetUnresolvedDependList();
    LPSERVICE_RECORD Service = ScGetServiceDatabase();
    LPDEPEND_RECORD Start;


    //
    // Search the unresolved depend list for a matching entry
    //
    while (UnresolvedEntry != NULL) {

        if (_wcsicmp(UnresolvedEntry->Name, DependOnService->ServiceName) == 0) {
            SC_LOG1(DEPEND, "Found unresolved entry for " FORMAT_LPWSTR "\n",
                    DependOnService->ServiceName);
            break;
        }

        UnresolvedEntry = UnresolvedEntry->Next;
    }

    if (UnresolvedEntry == NULL) {
        //
        // There are no service which depends on the service we are
        // installing; hence, no unresolved dependency to resolve.
        //
        SC_LOG1(DEPEND, "No service depends on " FORMAT_LPWSTR "\n",
                DependOnService->ServiceName);
        return NO_ERROR;
    }


    //
    // Loop through all services to see if any of them has a start depend
    // entry that points to UnresolvedEntry.
    //
    while (Service != NULL && UnresolvedEntry != NULL) {

        Start = Service->StartDepend;

        while (Start != NULL) {

            if (Start->DependUnresolved == UnresolvedEntry) {

                status = ScSetServiceDependList(
                             Start,
                             Service,
                             (PVOID)DependOnService,
                             TypeDependOnService
                             );

                if (status != NO_ERROR) {
                    //
                    // Error with creating the stop depend entry for
                    // DependOnService.  Back out changes set for the
                    // current start depend entry.
                    //
                    Start->DependType = TypeDependOnUnresolved;
                    Start->DependUnresolved = UnresolvedEntry;

                    //
                    // Back out of all other resolved dependencies to
                    // DependOnService will be done in ScDecrementUseCountAndDelete.
                    //
                    SC_LOG2(ERROR, "ScResolvedDependencyToService " FORMAT_LPWSTR
                            " failed " FORMAT_DWORD "\n",
                            DependOnService->ServiceName, status);
                    return status;
                }

                SC_LOG2(DEPEND, FORMAT_LPWSTR " depends on " FORMAT_LPWSTR
                        ".  Dependency resolved!\n", Service->ServiceName,
                        UnresolvedEntry->Name);

                ScDeleteUnresolvedDepend(&UnresolvedEntry);
            }

            Start = Start->Next;
        }

        Service = Service->Next;
    }

    return NO_ERROR;
}


DWORD
ScCreateDependencies(
    OUT PSERVICE_RECORD ServiceRecord,
    IN  LPWSTR Dependencies OPTIONAL
    )
/*++

Routine Description:

    This function creates the start dependencies list of a service.
    If the service specified by ServiceRecord depends on a service that
    has not been inserted into the service list yet, that service entry
    will be created and inserted into the service list so that the depend
    record can point to it.  The service this one points to in its start
    dependency list will get a new entry in its stop dependency list because
    this one must be stopped before it can stop.

    The dependencies list is not ordered.

    NOTE: This function is for call from RChangeServiceConfig.

Arguments:

    ServiceRecord - Receives the start dependencies information in this
        service record.

    Dependencies - Supplies the string which contains the names this service
        depend on to be started first.  This is a multi-sz string of
        service or group names.

Return Value:

    NO_ERROR - The operation was successful.

    ERROR_NOT_ENOUGH_MEMORY - Fail to allocate memory for required data
        structures.

Note:

    This routine assumes that the caller has exclusively acquired the
    database lock.  It is called by ScSetDependencyPointers.

    It also assumes that the caller has exclusively acquired the group
    list lock.

--*/
{
    DWORD status;

    LPWSTR DependPtr = Dependencies;
    LPWSTR DependOnName;
    PVOID DependOnRecord;
    DEPEND_TYPE Type;

    PDEPEND_RECORD Start;


    if (! ARGUMENT_PRESENT(DependPtr)) {
        return NO_ERROR;
    }

    while (*DependPtr != 0) {

        if (ScGetToken(&DependPtr, &DependOnName)) {

            //
            // Initialize flag for noting that a new service record is
            // created for resolving the dependency chain.
            //
            Type = 0;

            if (*DependOnName != SC_GROUP_IDENTIFIERW) {

                //
                // Depend on a service
                //

                //
                // Look for the service we depend on in the service record list
                //
                status = ScGetNamedServiceRecord(
                             DependOnName,
                             (LPSERVICE_RECORD *) &DependOnRecord
                             );

                if (status == ERROR_SERVICE_DOES_NOT_EXIST) {

                    //
                    // Could not find the service we depend on.  Create an
                    // unresolved dependency entry.
                    //
                    status = ScCreateUnresolvedDepend(
                                DependOnName,
                                (PUNRESOLVED_DEPEND *) &DependOnRecord
                                );

                    if (status != NO_ERROR) {
                        goto ErrorExit;
                    }

                    //
                    // New unresolved depend entry created.  We have to remove
                    // it if any error occurs later.
                    //
                    Type = TypeDependOnUnresolved;

                }
                else {

                    Type = TypeDependOnService;
                }

                if (status != NO_ERROR) {
                    goto ErrorExit;
                }
            }
            else {

                //
                // Depend on a group
                //

                PLOAD_ORDER_GROUP GroupEntry = ScGetOrderGroupList();

                DependOnName++;

                //
                // Search for matching group name in load order list
                //
                while (GroupEntry != NULL) {
                    if (_wcsicmp(GroupEntry->GroupName, DependOnName) == 0) {
                        DependOnRecord = (PVOID) GroupEntry;
                        Type = TypeDependOnGroup;
                        break;
                    }
                    GroupEntry = GroupEntry->Next;
                }

                if (GroupEntry == NULL) {
                    //
                    // Could not find group in the OrderGroup list.  Must
                    // depend on a standalone group.
                    //
                    status = ScCreateStandaloneGroup(
                                 DependOnName,
                                 (LPLOAD_ORDER_GROUP *) &DependOnRecord
                                 );

                    if (status != NO_ERROR) {
                        goto ErrorExit;
                    }

                    Type = TypeDependOnGroup;
                }
            }

            //
            // Allocate memory for start depend record and insert it in the
            // front of the start depend list of the service we are processing.
            //
            if ((status = ScCreateDependRecord(
                              TRUE,              // For start list
                              ServiceRecord,
                              &Start
                              )) != NO_ERROR) {

                goto ErrorExit;
            }

            //
            // Start depend record created OK, set fields.  Set stop
            // depend if appropriate (Type == TypeDependOnService).
            //
            status = ScSetServiceDependList(
                         Start,
                         ServiceRecord,
                         DependOnRecord,
                         Type
                         );

            if (status != NO_ERROR) {

                //
                // Remove the start depend record just created in the front of
                // the start depend list and delete it.
                //
                ServiceRecord->StartDepend = Start->Next;
                (void) LocalFree((HLOCAL)Start);

                goto ErrorExit;
            }

        } // if got token

    }  // while there is a dependency

    return NO_ERROR;

ErrorExit:

    //
    // Remove newly created service record because of errors and we cannot
    // proceed.
    //
    if (Type == TypeDependOnUnresolved) {

        ScDeleteUnresolvedDepend((PUNRESOLVED_DEPEND *) &DependOnRecord);
    }

    //
    // Clean up dependencies created up to the point of failure
    //
    ScDeleteStartDependencies(ServiceRecord);

    return status;
}


DWORD
ScCreateUnresolvedDepend(
   IN LPWSTR Name,
   OUT LPUNRESOLVED_DEPEND *Unresolved
   )
/*++

Routine Description:

    This function looks for a matching unresolved entry in the unresolved
    depend list.  If a match is found, the reference count is incremented
    and the pointer to the matching entry is returned.

    If no matching entry is found, this function creates a new unresolved
    entry, insert it into the end of the unresolved depend list, and
    return a pointer to the new entry.


Arguments:

    Name - Supplies the name of the service or group which has not been
        installed yet, and thus needing this unresolved depend entry.

    Unresolved - Receives a pointer to the unresolved entry.

Return Value:

    NO_ERROR - The operation was successful.

    ERROR_NOT_ENOUGH_MEMORY - Allocation of memory failed.

Note:

    This routine assumes that the caller has exclusively acquired the
    database lock.  It is called by ScCreateDependencies.

--*/
{

    LPUNRESOLVED_DEPEND UnresolvedList = ScGetUnresolvedDependList();

    BOOL Found = FALSE;


    //
    // Search the existing unresolved depend list for the matching
    // unresolved depend entry.
    //
    while (UnresolvedList != NULL) {

        if (_wcsicmp(UnresolvedList->Name, Name) == 0) {
            Found = TRUE;
            break;
        }

        UnresolvedList = UnresolvedList->Next;
    }

    if (Found) {

        UnresolvedList->RefCount++;

        SC_LOG2(DEPEND,
            "Found existing unresolved entry for " FORMAT_LPWSTR
            ", just increment refcount to %lu\n", UnresolvedList->Name,
            UnresolvedList->RefCount);

        *Unresolved = UnresolvedList;

        return NO_ERROR;
    }

    //
    // Not found.  Allocate a new unresolved entry.
    //
    if ((*Unresolved = (LPUNRESOLVED_DEPEND)LocalAlloc(
                             LMEM_ZEROINIT,
                             sizeof(UNRESOLVED_DEPEND) + WCSSIZE(Name)
                             )) == NULL) {
        SC_LOG1(ERROR,"ScCreateUnresolvedDepend: LocalAlloc failure %lu\n",
                GetLastError());
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    (*Unresolved)->Name = (LPWSTR) ((DWORD) *Unresolved +
                                    sizeof(UNRESOLVED_DEPEND));
    wcscpy((*Unresolved)->Name, Name);

    (*Unresolved)->RefCount = 1;

    SC_LOG1(DEPEND, "Created new unresolved depend entry "
            FORMAT_LPWSTR "\n", (*Unresolved)->Name);

    UnresolvedList = &UnresolvedDependList;

    //
    // Add the new unresolved entry to the unresolved list at the end.
    //
    ADD_TO_LIST(UnresolvedList, *Unresolved);

    return NO_ERROR;
}


VOID
ScDeleteUnresolvedDepend(
    IN OUT LPUNRESOLVED_DEPEND *Unresolved
    )
{

    if ((*Unresolved)->RefCount) {
        (*Unresolved)->RefCount--;
        SC_LOG1(DEPEND, "ScDeleteUnresolvedDepend: Subtracted RefCount is "
                FORMAT_DWORD "\n", (*Unresolved)->RefCount);
    }
    else {
        //
        // The reference count better not be 0.
        //
        SC_LOG0(ERROR, "ScDeleteUnresolvedDepend: Before delete, refcount is 0!\n");
        SC_ASSERT(FALSE);
    }

    if ((*Unresolved)->RefCount == 0) {
        REMOVE_FROM_LIST(*Unresolved);
        (void) LocalFree((HLOCAL)*Unresolved);
        *Unresolved = NULL;
    }
}


DWORD
ScCreateDependRecord(
    IN  BOOL IsStartList,
    IN  OUT PSERVICE_RECORD ServiceRecord,
    OUT PDEPEND_RECORD *DependRecord
    )
/*++

Routine Description:

    This function allocates the memory for a depend record, and insert
    it into the front of the specific depend list.  If IsStartList is
    TRUE, the depend record goes into the start depend list of
    ServiceRecord, otherwise the depend record goes into the stop
    depend list of the ServiceRecord.

Arguments:

    IsStartList - TRUE indicates to insert into start list, FALSE indicates
        to insert into stop list.

    ServiceRecord - Receives the start/stop depend record in its dependency
        list.

    DependRecord - Receives a pointer to the new depend record created.

Return Value:

    NO_ERROR - The operation was successful.

    ERROR_NOT_ENOUGH_MEMORY - The call to allocate memory for a depend
        record failed.

Note:

    This routine assumes that the caller has exclusively acquired the
    database lock.  It is called by ScCreateDependencies.

--*/
{

    if ((*DependRecord = (PDEPEND_RECORD)LocalAlloc(
                             LMEM_ZEROINIT,
                             sizeof(DEPEND_RECORD)
                             )) == NULL) {
        SC_LOG(ERROR,"ScCreateDependRecord: LocalAlloc failure %ld\n",
               GetLastError());
        return ERROR_NOT_ENOUGH_MEMORY;
    }


    //
    // Insert the depend record into the front of the list
    //
    if (IsStartList) {
        //
        // Start depend
        //
        (*DependRecord)->Next = ServiceRecord->StartDepend;
        ServiceRecord->StartDepend = (PVOID) (*DependRecord);
    }
    else {
        //
        // Stop depend
        //
        (*DependRecord)->Next = ServiceRecord->StopDepend;
        ServiceRecord->StopDepend = *DependRecord;
    }

    return NO_ERROR;
}


VOID
ScDeleteStartDependencies(
    IN PSERVICE_RECORD ServiceRecord
    )
/*++

Routine Description:

    This function deletes the start dependencies list of a service.  It also
    deletes the the stop dependencies of other services which need this
    service to be stopped first.

    NOTE: This function is for call from RChangeServiceConfig.

Arguments:

    ServiceRecord - Supplies the start dependencies information in this
        service record.

Return Value:

    None.

Note:

    This routine assumes that the caller has exclusively acquired the
    database lock.  It is called by ScAddConfigInfoServiceRecord and
    ScDecrementUseCountAndDelete.

    It also assumes that the caller has exclusively acquired the group
    list lock.

--*/
{
    PDEPEND_RECORD StartEntry;
    PDEPEND_RECORD StopEntry;
    PDEPEND_RECORD StopBackPointer;


    StartEntry = ServiceRecord->StartDepend;

    while (StartEntry != NULL) {

        if (StartEntry->DependType == TypeDependOnService) {

            LPSERVICE_RECORD DependencyService = StartEntry->DependService;


            //
            // Find the stop depend record for the service which depends on this
            // service to be stopped first, and delete it.
            //
            StopEntry = DependencyService->StopDepend;
            StopBackPointer = StopEntry;

            while ((StopEntry != NULL) &&
                   (StopEntry->DependService != ServiceRecord)) {

                StopBackPointer = StopEntry;
                StopEntry = StopEntry->Next;
            }

            if (StopEntry == NULL) {
#ifndef _CAIRO_
                //
                // We allow Netlogon to appear in the start dependency list, but
                // not in the stop dependency list.  This is for the case where
                // we add a "soft" dependency on netlogon because the service runs
                // in a remove account.
                //
                if (_wcsicmp(DependencyService->ServiceName,L"Netlogon") != 0) {
#endif // _CAIRO_
                    SC_LOG1(ERROR,
                            "ScDeleteStartDependencies: Failed to find matching stop depend node for "
                            FORMAT_LPWSTR "\n",
                            DependencyService->ServiceName);
                    SC_ASSERT(FALSE);
                    return;
#ifndef _CAIRO_
                }
#endif // _CAIRO_
            }
            else {

                if (StopEntry->DependService == ServiceRecord) {

                    if ((PVOID) StopBackPointer == StopEntry) {
                        //
                        // Unchaining from the front of the list
                        //
                        DependencyService->StopDepend = StopEntry->Next;
                    }
                    else {
                        //
                        // Unchaining from the middle or end of the list
                        //
                        StopBackPointer->Next = StopEntry->Next;
                    }

                    (void) LocalFree((HLOCAL)StopEntry);
                }
            }
        }
        else if (StartEntry->DependType == TypeDependOnGroup) {

            //
            // Decrement the reference count on the standalone group
            // entry and deletes it if 0.
            //
            if (StartEntry->DependGroup->RefCount != MAXULONG) {
                ScDeleteStandaloneGroup(StartEntry->DependGroup);
            }

        }
        else {

            //
            // Dependency type is unresolved.
            //
            ScDeleteUnresolvedDepend(&StartEntry->DependUnresolved);
        }

        //
        // Now delete the start depend record.
        //
        ServiceRecord->StartDepend = StartEntry->Next;
        (void) LocalFree((HLOCAL)StartEntry);
        StartEntry = ServiceRecord->StartDepend;
    }
}


VOID
ScDeleteStopDependencies(
    IN PSERVICE_RECORD ServiceToBeDeleted
    )
/*++

Routine Description:

    This function deletes the stop dependencies list of a service.  For
    every stop depend service, it makes the start depend pointer of that
    service to point to an unresolved depend entry.

    This function is called when the service is to be deleted.

Arguments:

    ServiceToBeDeleted - Supplies the pointer to the service that will
        be deleted.

Return Value:

    None.

Note:

    This routine assumes that the caller has exclusively acquired the
    database lock.  It is called by ScDecrementUseCountAndDelete.

--*/
{

    DWORD status;
    PDEPEND_RECORD StartEntry;
    PDEPEND_RECORD StopEntry;
    LPUNRESOLVED_DEPEND Unresolved;


    StopEntry = ServiceToBeDeleted->StopDepend;

    while (StopEntry != NULL) {

        LPSERVICE_RECORD DependencyService = StopEntry->DependService;

        //
        // Loop through the start depend entries of the service which
        // depends on ServiceToBeDeleted.
        //
        StartEntry = DependencyService->StartDepend;

        while (StartEntry != NULL) {

            if (StartEntry->DependService == ServiceToBeDeleted) {
                break;
            }

            StartEntry = StartEntry->Next;
        }

        if (StartEntry != NULL) {

            //
            // Found a start depend entry that points to the service to be.
            // deleted.  Make it point to an unresolved depend entry that
            // represents that service.
            //
            status = ScCreateUnresolvedDepend(
                         ServiceToBeDeleted->ServiceName,
                         &Unresolved
                         );

            if (status == NO_ERROR) {
                StartEntry->DependType = TypeDependOnUnresolved;
                StartEntry->DependUnresolved = Unresolved;
            }
        }

        //
        // Now delete the start depend record.
        //
        ServiceToBeDeleted->StopDepend = StopEntry->Next;
        (void) LocalFree((HLOCAL)StopEntry);
        StopEntry = ServiceToBeDeleted->StopDepend;
    }
}


DWORD
ScSetServiceDependList(
    LPDEPEND_RECORD Start,
    LPSERVICE_RECORD ServiceRecord,
    PVOID DependOnRecord,
    DEPEND_TYPE DependOnType
    )
/*++

Routine Description:

    This function

Arguments:

Return Value:

    None.

Note:

    This routine assumes that the caller has exclusively acquired the
    database lock.  It is called by ScResolveDependencyToService and
    ScCreateDependencies.

--*/
{
    DWORD status;
    LPDEPEND_RECORD Stop;


    //
    // Set fields for start depend entry.
    //
    Start->DependType = DependOnType;
    Start->Depend = DependOnRecord;

    if (DependOnType == TypeDependOnService) {

        //
        // Allocate memory for stop depend record and insert it in the
        // front of the stop depend list of the service we depend on.
        //
        if ((status = ScCreateDependRecord(
                          FALSE,             // For stop list
                          (LPSERVICE_RECORD) DependOnRecord,
                          &Stop
                          )) != NO_ERROR) {

            return status;
        }

        Stop->DependType = TypeDependOnService;
        Stop->DependService = ServiceRecord;
    }

    return NO_ERROR;
}


VOID
ScChangeGroupOrder(
    LPWSTR Groups
    )
/*++

Routine Description:

    This function is called by ScHandleGroupOrderChange which is triggered
    by a change notification on the ServiceGroupOrder key in the registy.
    The new ServiceGroupOrder list (REG_MULTI_SZ) is the input parameter
    to this function.

Arguments:

    Groups - Supplies the new ServiceGroupOrder list from the registry.
        This is a NULL-NULL string.

Return Value:

    None.

Note:

    The GroupListLock must be held exclusively prior to calling this routine.

--*/
{
    LPWSTR GroupName;
    LPWSTR GroupPtr = Groups;
    LPLOAD_ORDER_GROUP Done = &OrderGroupList;
    LPLOAD_ORDER_GROUP OrderEntry;
    LPLOAD_ORDER_GROUP StandaloneEntry;


    OrderEntry = ScGetOrderGroupList();

    while (*GroupPtr != 0) {

        if (ScGetToken(&GroupPtr, &GroupName)) {

            if (OrderEntry == NULL) {

                SC_LOG0(DEPEND, "ScChangeGroupOrder: OrderEntry is NULL\n");

                //
                // Reached the end of the order group list.  Look for
                // the group name in the standalone list.
                //
                StandaloneEntry = ScFindStandaloneGroup(GroupName);

                if (StandaloneEntry != NULL) {

                    //
                    // Add the standalone group to the end of the order
                    // group list.
                    //
                    Done = &OrderGroupList;
                    ADD_TO_LIST(Done, StandaloneEntry);

                }
                else {

                    //
                    // Standalone group not found.  Create a new entry
                    // and add it to the end of the order group list.
                    //
                    (void) ScCreateOrderGroupEntry(GroupName);
                }

            }
            else {

                SC_LOG1(DEPEND, "ScChangeGroupOrder: OrderEntry is "
                        FORMAT_LPWSTR "\n", OrderEntry->GroupName);

                if (_wcsicmp(OrderEntry->GroupName, GroupName) != 0) {


                    //
                    // Search for the non-matching group name in the
                    // group order list.  Perhaps we are trying to move
                    // the group up in the order chain.
                    //
                    OrderEntry = ScFindOrderGroup(
                                     OrderEntry->Next,
                                     GroupName
                                     );


                    if (OrderEntry == NULL) {

                        //
                        // No such group in the order list.  Look for the
                        // group in standalone list.
                        //
                        OrderEntry = ScFindStandaloneGroup(GroupName);

                        if (OrderEntry == NULL) {

                            //
                            // Could not find group in the standalone list
                            // either.  Allocate a new group entry.
                            //
                            (void) ScAllocateOrderGroupEntry(
                                       &OrderEntry,
                                       GroupName
                                       );

                        }
                    }

                    SC_ASSERT(OrderEntry);

                    if (OrderEntry != NULL) {

                        //
                        // Insert the entry into the order group list.
                        //

                        OrderEntry->Next = Done->Next;

                        if (Done->Next != NULL) {
                            Done->Next->Prev = OrderEntry;
                        }

                        Done->Next = OrderEntry;
                        OrderEntry->Prev = Done;
                    }

                }
                else {
                    SC_LOG1(DEPEND, "Group entry " FORMAT_LPWSTR
                            " stays the same\n", GroupName);
                }

                //
                // The Done and OrderEntry pointers are updated the same
                // way whether the two group names matches or not.
                //
                Done = OrderEntry;

                OrderEntry = OrderEntry->Next;
            }

        }

    } // while *GroupPtr != 0


    //
    // Every remaining order group entry gets moved to the standalone
    // list.  Update the refcount by finding the number of members in
    // this group as well as dependencies on this group.
    //
    while (OrderEntry != NULL) {

        SC_LOG1(DEPEND, "ScChangeGroupOrder: Remaining OrderEntry is "
                FORMAT_LPWSTR "\n", OrderEntry->GroupName);

        StandaloneEntry = OrderEntry;

        OrderEntry = OrderEntry->Next;

        //
        // Remove the group from the order list because it is no longer
        // specified in the new ServiceGroupOrder list.  Add it to the
        // standalone group list at the end.
        //
        REMOVE_FROM_LIST(StandaloneEntry);

        ScSetStandaloneGroupRefcount(StandaloneEntry);

        if (StandaloneEntry->RefCount == 0) {
            ScForgetSpecialGroup(StandaloneEntry);
            (VOID) LocalFree((HLOCAL)StandaloneEntry);
        }
        else {
            Done = &StandaloneGroupList;
            ADD_TO_LIST(Done, StandaloneEntry);
        }
    }

}


LPLOAD_ORDER_GROUP
ScFindStandaloneGroup(
    IN LPWSTR GroupName
    )
/*++

Routine Description:

    This function searches for a group in the standalone group list
    to be added to the order group list.

    If found, the group is removed from the list, its refcount is set
    to MAXULONG, and its pointer is returned.

Arguments:

    GroupName - Supplies the name of the group to look for.

Return Value:

    Returns the pointer to group found.  If not found, this value is
        NULL.

Note:

    This routine assumes that the caller has exclusively acquired the
    group list lock.  It is called by ScChangeGroupOrder.

--*/
{
    LPLOAD_ORDER_GROUP Group;


    Group = ScGetStandaloneGroupList();

    while (Group != NULL) {

        if (_wcsicmp(Group->GroupName, GroupName) == 0) {

            SC_LOG1(DEPEND, "ScFindStandaloneGroup: Found standalone entry for "
                    FORMAT_LPWSTR "\n", GroupName);
            break;
        }

        Group = Group->Next;
    }

    if (Group != NULL) {

        //
        // Found the standalone group.  Change its RefCount since it is
        // about to become an order group.
        //
        REMOVE_FROM_LIST(Group);
        Group->RefCount = MAXULONG;

    }
    else {
        SC_LOG1(DEPEND, "ScFindStandaloneGroup: Didn't find " FORMAT_LPWSTR
                "\n", GroupName);
    }

    return Group;
}


LPLOAD_ORDER_GROUP
ScFindOrderGroup(
    IN LPLOAD_ORDER_GROUP Group,
    IN LPWSTR GroupName
    )
/*++

Routine Description:

    This function searches for a group in the order group list, from a
    specific starting point in the list.

    If found, the group is removed from the list, and its pointer is
    returned.

Arguments:

    Group - Supplies a pointer to the list to start the search.

    GroupName - Supplies the name of the group to look for.

Return Value:

    Returns the pointer to group found.  If not found, this value is
        NULL.

Note:

    This routine assumes that the caller has exclusively acquired the
    group list lock.  It is called by ScChangeGroupOrder.

--*/

{
    while (Group != NULL) {

        if (_wcsicmp(Group->GroupName, GroupName) == 0) {

            SC_LOG1(DEPEND, "ScFindOrderGroup: Found order entry for "
                    FORMAT_LPWSTR "\n", GroupName);
            break;
        }

        Group = Group->Next;
    }

    if (Group != NULL) {

        //
        // Found the order group.  Change its RefCount since it is
        // about to become an order group.
        //
        REMOVE_FROM_LIST(Group);

    }
    else {
        SC_LOG1(DEPEND, "ScFindOrderGroup: Didn't find " FORMAT_LPWSTR
                "\n", GroupName);
    }

    return Group;
}


LPLOAD_ORDER_GROUP
ScGetNamedGroupRecord(
    IN LPCWSTR GroupName
    )
/*++

Routine Description:

    This function searches for a named group, first in the order group
    list and next in the standalone group list.

Arguments:

    GroupName - Supplies the name of the group to look for.

Return Value:

    Returns the pointer to group found.  If not found, this value is
        NULL.

Note:

    This routine assumes that the caller has exclusively acquired the
    group list lock.

--*/
{
    LPLOAD_ORDER_GROUP Group;

    for (Group = ScGetOrderGroupList();
         Group != NULL;
         Group = Group->Next)
    {
        if (_wcsicmp(Group->GroupName, GroupName) == 0)
        {
            break;
        }
    }

    if (Group == NULL)
    {
        for (Group = ScGetStandaloneGroupList();
             Group != NULL;
             Group = Group->Next)
        {
            if (_wcsicmp(Group->GroupName, GroupName) == 0)
            {
                break;
            }
        }
    }

    return Group;
}


VOID
ScSetStandaloneGroupRefcount(
    IN LPLOAD_ORDER_GROUP StandaloneGroup
    )
/*++

Routine Description:

    This function goes through all services and looks for members of
    the specified standalone group.  Whenever a member of this group
    is found, or whenever the RegistryGroup pointer points to this
    standalone group, the refcount of the group is incremented.

    Also, if any service depends on the specified standalone group,
    the refcount of the group is incremented.

Arguments:

    StandaloneGroup - Supplies a pointer to the standalone group whose
        refcount is to be set to the appropriate value.

Return Value:

    None.

Note:

    This routine assumes that the caller has exclusively acquired the
    group list lock.  It is called by ScChangeGroupOrder.

--*/
{

    LPSERVICE_RECORD Service = ScGetServiceDatabase();
    LPDEPEND_RECORD Start;


    StandaloneGroup->RefCount = 0;

    while (Service != NULL) {

        if (Service->MemberOfGroup == StandaloneGroup) {
            StandaloneGroup->RefCount++;
        }

        if (Service->RegistryGroup == StandaloneGroup) {
            StandaloneGroup->RefCount++;
        }

        Start = Service->StartDepend;

        while (Start != NULL) {

            if ((Start->DependType == TypeDependOnGroup) &&
                (Start->DependGroup == StandaloneGroup)) {

                StandaloneGroup->RefCount++;
            }

            Start = Start->Next;
        }

        Service = Service->Next;
    }

    SC_LOG2(DEPEND, "ScSetStandaloneGroupRefcount: Group " FORMAT_LPWSTR
            " has refcount %lu\n", StandaloneGroup->GroupName,
            StandaloneGroup->RefCount);
}


VOID
ScGetUniqueTag(
    IN  LPWSTR GroupName,
    OUT LPDWORD Tag
    )
/*++

Routine Description:

    This function looks for a unique tag value within the specified
    group.

Arguments:

    GroupName - Specifies the group name within which the value tag
        returned must be unique.

    Tag - Receives the unique tag value.

Return Value:

    None.

Note:

    This function acquires share access to the group list lock.

    It assumes that the exclusive service database lock is already
    acquired so that no other caller can execute this code until
    the returned tag is fully assigned to the service, and that the
    service entries in the database list don't change.

    The GroupListLock must be held exclusively prior to calling this
    function.

--*/
{
    LPDWORD TagArray;
    DWORD TagArrayLength;

    DWORD ReturnTag = 1;

    LPLOAD_ORDER_GROUP Group;
    LPSERVICE_RECORD Service;


    if (ScGetGroupVector(
            GroupName,
            (LPBYTE *) &TagArray,
            &TagArrayLength
            ) == NO_ERROR) {

        if (TagArray != NULL) {
            //
            // The returned values is actually the number of bytes.  Divide it
            // by size of DWORD to make it the number of array entries.
            //
            TagArrayLength = TagArrayLength / sizeof(DWORD);

            SC_ASSERT((TagArrayLength - 1) == TagArray[0]);

            ScCompareVector(
                TagArray,
                TagArrayLength,
                &ReturnTag
                );
        }
    }
    else {

        TagArray = NULL;
    }

    Group = ScGetNamedGroupRecord(GroupName);

    if (Group != NULL) {
GroupAgain:
        Service = ScGetServiceDatabase();

        while (Service != NULL) {

            if ((Service->RegistryGroup == Group) &&
                (Service->Tag == ReturnTag)) {

                ReturnTag++;

                if (TagArray != NULL) {

                    ScCompareVector(
                        TagArray,
                        TagArrayLength,
                        &ReturnTag
                        );
                }

                goto GroupAgain;
            }

            Service = Service->Next;

        } // while all services
    }


    *Tag = ReturnTag;

    SC_LOG(DEPEND, "ScGetUniqueTag: Tag=" FORMAT_DWORD "\n", *Tag);

}


VOID
ScCompareVector(
    IN     LPDWORD TagArray,
    IN     DWORD TagArrayLength,
    IN OUT LPDWORD ReturnTagPtr
    )
{
    DWORD i;

VectorAgain:
    for (i = 1; i < TagArrayLength; i++) {

        if (TagArray[i] == (*ReturnTagPtr)) {

            SC_LOG(DEPEND_DUMP, "Tag " FORMAT_DWORD " is not unique\n",
                   *ReturnTagPtr);

            (*ReturnTagPtr)++;
            goto VectorAgain;
        }
    }
}

VOID
ScGetDependencySize(
    LPSERVICE_RECORD    ServiceRecord,
    LPDWORD             DependSize,
    LPDWORD             MaxDependSize
    )

/*++

Routine Description:



Arguments:

    ServiceRecord -

    DependSize - This points to a location that will contain the number
        of bytes required for the list of dependency strings.

    MaxDependSize - This points to a location that will contain the
        number of bytes in the longest dependency string in the set.

Return Value:



--*/
{
    LPDEPEND_RECORD     dependRecord;
    DWORD               bytesNeeded = 0;
    DWORD               StrSize=0;

    dependRecord = ServiceRecord->StartDepend;

    //
    // NOTE: Dependencies are expected to be a double NULL terminated
    //       terminated set of strings.
    //
    bytesNeeded += sizeof(WCHAR);

    if (dependRecord == NULL) {
        bytesNeeded += sizeof(WCHAR);
    }
    while (dependRecord != NULL) {

        SC_ASSERT( dependRecord->Depend != NULL );

        // Add room.  WCSSIZE adds 1 char.  For final entry,  we'll
        // use null char.  In between, we'll put some separator.

        if (dependRecord->DependType == TypeDependOnService) {
            StrSize =
                WCSSIZE(dependRecord->DependService->ServiceName); // sizes...

        }
        else if (dependRecord->DependType == TypeDependOnGroup) {
            StrSize =
               WCSSIZE(dependRecord->DependGroup->GroupName) +
               sizeof(WCHAR); // name size plus SC_GROUP_IDENTIFIERW
        }
        else {
            //
            // Unresolved service dependency
            //
            StrSize = WCSSIZE(dependRecord->DependUnresolved->Name);
        }

        bytesNeeded += StrSize;

        if (StrSize > *MaxDependSize) {
            *MaxDependSize = StrSize;
        }

        dependRecord = dependRecord->Next;
    }
    *DependSize = bytesNeeded;
}

DWORD
ScGetDependencyString(
    LPSERVICE_RECORD    ServiceRecord,
    DWORD               MaxDependSize,
    DWORD               DependSize,
    LPWSTR              lpDependencies
    )

/*++

Routine Description:



Arguments:

    ServiceRecord -

    MaxDependSize - This is the size of the largest string in the
        dependency list.

    lpDependencies - This is a pointer to the location where the
        list of dependency strings is to be stored.

Return Value:



--*/
{
    LPVOID          endOfVariableData;
    LPVOID          fixedDataEnd;
    LPDEPEND_RECORD dependRecord;
    DWORD           bufSize;
    DWORD           ApiStatus = NO_ERROR;

    //
    // Put dependencies in the return buffer.  Since it is a NULL-NULL
    // string, put an extra NULL at the end to begin with.
    //
    endOfVariableData = ((LPBYTE)lpDependencies) + DependSize;

    endOfVariableData = (LPVOID) ((DWORD) endOfVariableData - sizeof(WCHAR));
    * ((LPWSTR) endOfVariableData) = L'\0';

    fixedDataEnd = lpDependencies;

    dependRecord = ServiceRecord->StartDepend;

    if (dependRecord == NULL) {
        //
        // If there are no dependencies, then we need to add a seperator
        // that will be followed by the NULL (immediately above).
        // This seperator is used to get us across the RPC interface.
        // Then on the client side, it is changed to a NULL.  So we end
        // up with an empty-double-NULL-terminated-string.
        //
        endOfVariableData = (LPVOID) ((DWORD) endOfVariableData - sizeof(WCHAR));
        * ((LPWSTR) endOfVariableData) = L'/';
        lpDependencies = endOfVariableData;
    }
    else {

        LPWSTR DependName;


        DependName = (LPWSTR)LocalAlloc(0, (UINT) MaxDependSize);

        if (DependName == NULL) {
            ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        lpDependencies = (LPWSTR) endOfVariableData;
        while (dependRecord != NULL) {

            SC_ASSERT( dependRecord->Depend != NULL );


            if (dependRecord->DependType == TypeDependOnService) {

                wcscpy(DependName, dependRecord->DependService->ServiceName);

            }
            else if (dependRecord->DependType == TypeDependOnGroup) {

                *DependName = SC_GROUP_IDENTIFIERW;

                wcscpy(DependName + 1, dependRecord->DependGroup->GroupName);
            }
            else {
                //
                // Unresolved service dependency
                //
                wcscpy(DependName, dependRecord->DependUnresolved->Name);
            }

            bufSize = wcslen(DependName);

            if ( !ScCopyStringToBufferW (
                    DependName,
                    bufSize,
                    fixedDataEnd,
                    (LPWSTR *) & endOfVariableData,
                    &lpDependencies
                    ) ) {

                SC_LOG0(ERROR,
                    "RQueryServiceConfigW:ScCopyStringtoBufferW (Dependencies)Failed\n");

                SC_ASSERT( FALSE );
                ApiStatus = ERROR_INSUFFICIENT_BUFFER;
                (VOID) LocalFree((HLOCAL)DependName);
                goto Cleanup;
            }
            else {
                //
                // Add separator character.
                //

                lpDependencies[bufSize] = L'/';
            }

            dependRecord = dependRecord->Next;
        }

        (VOID) LocalFree((HLOCAL)DependName);
    }
Cleanup:
    return(ApiStatus);
}


#if DBG
VOID
ScDumpGroups(
    VOID
    )
/*++

Routine Description:

    This function walks group list prints out each entry.

Arguments:

    None.

Return Value:

    None.

Note:

    Calls to this routine must be enclosed within #if DBG.

--*/
{
    PLOAD_ORDER_GROUP GroupEntry = ScGetOrderGroupList();


    while (GroupEntry != NULL) {
        DbgPrint("\nOrdered Groups:\n");
        DbgPrint("Group: Handle=%08lx Name=%ws RefCount=x%lx\n", GroupEntry,
                 GroupEntry->GroupName, GroupEntry->RefCount);
        GroupEntry = GroupEntry->Next;
    }

    GroupEntry = ScGetStandaloneGroupList();

    while (GroupEntry != NULL) {
        DbgPrint("Standalone Groups:\n");
        DbgPrint("Group: Handle=%08lx Name=%ws RefCount=x%lx\n", GroupEntry,
                 GroupEntry->GroupName, GroupEntry->RefCount);
        GroupEntry = GroupEntry->Next;
    }

    DbgPrint("\nTDI     group is at %08lx"
             "\nPNP_TDI group is at %08lx",
             "\nNDIS    group is at %08lx",
             "\nNDISWAN group is at %08lx\n",
             ScGlobalTDIGroup, ScGlobalPNP_TDIGroup, ScGlobalNDISGroup,
             ScGlobalNDISWANGroup);
}

VOID
ScDumpServiceDependencies(
    VOID
    )
/*++

Routine Description:

    This function walks the start and stop dependencies lists of every
    service in the service record list.

Arguments:

    None.

Return Value:

    None.

Note:

    Calls to this routine must be enclosed within #if DBG.

--*/
{
    PSERVICE_RECORD ServiceRecord = ScGetServiceDatabase();
    PDEPEND_RECORD DependList;


    while (ServiceRecord != NULL) {

        DbgPrint("Service: %-20ws UseCount=%lu",
            ServiceRecord->ServiceName, ServiceRecord->UseCount);

        DbgPrint(" MemberOfGroup=%08lx ", ServiceRecord->MemberOfGroup);

        if (ServiceRecord->MemberOfGroup != NULL) {
            if (ServiceRecord->MemberOfGroup->RefCount != MAXULONG) {
                DbgPrint("SG=%ws\n", ServiceRecord->MemberOfGroup->GroupName);
            }
            else if (ServiceRecord->MemberOfGroup->RefCount == MAXULONG) {
                DbgPrint("OG=%ws\n", ServiceRecord->MemberOfGroup->GroupName);
            }
        }
        else {
            DbgPrint("\n");
        }

        if (ServiceRecord->RegistryGroup != NULL) {
            DbgPrint("                    RG=%ws\n", ServiceRecord->RegistryGroup->GroupName);
        }

        //
        // Dump start depend
        //
        DependList = ServiceRecord->StartDepend;

        if (DependList != NULL) {
            DbgPrint("    StartDepend:\n");
        }

        while (DependList != NULL) {
            DbgPrint("        %ws\n", DependList->DependService->ServiceName);
            DependList = DependList->Next;
        }

        //
        // Dump stop depend
        //
        DependList = ServiceRecord->StopDepend;
        if (DependList != NULL) {
            DbgPrint("    StopDepend:\n");
        }

        while (DependList != NULL) {
            DbgPrint("        %ws\n", DependList->DependService->ServiceName);
            DependList = DependList->Next;
        }

        ServiceRecord = ServiceRecord->Next;
    }
}

#endif // #if DBG

