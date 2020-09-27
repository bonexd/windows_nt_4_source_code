/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

	ffilter.h

Abstract:

	Header file for the address filtering library for NDIS MAC's.

Author:

	Anthony V. Ercolano (tonye) creation-date 3-Aug-1990

Environment:

	Runs in the context of a single MAC driver.

Notes:

	None.

Revision History:

	Sean Selitrennikoff (SeanSe) - converter efilter.* for FDDI support.


--*/

#ifndef _FDDI_FILTER_DEFS_
#define _FDDI_FILTER_DEFS_

#define FDDI_LENGTH_OF_LONG_ADDRESS 6
#define FDDI_LENGTH_OF_SHORT_ADDRESS 2


//
// ZZZ This is a little-endian specific check.
//
#define FDDI_IS_MULTICAST(Address, AddressLength, Result) \
	*Result = (BOOLEAN)(*(UCHAR *)(Address) & (UCHAR)0x01)

//
// Check whether the frame is SMT or not.
//
#define FDDI_IS_SMT(FcByte, Result) \
{ \
	*Result = ((FcByte & ((UCHAR)0xf0)) == 0x40); \
}


//
// Check whether an address is broadcast.
//
#define FDDI_IS_BROADCAST(Address, AddressLength, Result) 	\
	*Result = (*(PUCHAR)(Address) == (UCHAR)0xFF)


//
// This macro will compare network addresses.
//
//  A - Is a network address.
//
//  B - Is a network address.
//
//  Result - The result of comparing two network address.
//
//  Result < 0 Implies the B address is greater.
//  Result > 0 Implies the A element is greater.
//  Result = 0 Implies equality.
//
// Note that this is an arbitrary ordering.  There is not
// defined relation on network addresses.  This is ad-hoc!
//
//
#define FDDI_COMPARE_NETWORK_ADDRESSES(_A, _B, _Length, _Result)	\
{																	\
	if (*(USHORT UNALIGNED *)(_A) >								 	\
		*(USHORT UNALIGNED *)(_B))									\
	{																\
		*(_Result) = 1;												\
	}																\
	else if (*(USHORT UNALIGNED *)(_A) <							\
			 *(USHORT UNALIGNED *)(_B))								\
	{																\
		*(_Result) = (UINT)-1;										\
	}																\
	else if (_Length == 2)											\
	{																\
		*(_Result) = 0;												\
	}																\
	else if (*(ULONG UNALIGNED *)((PUCHAR)(_A) + 2) >				\
			 *(ULONG UNALIGNED *)((PUCHAR)(_B) + 2))				\
	{																\
		*(_Result) = 1;												\
	}																\
	else if (*(ULONG UNALIGNED *)((PUCHAR)(_A) + 2) <				\
			 *(ULONG UNALIGNED *)((PUCHAR)(_B) + 2))				\
	{																\
		*(_Result) = (UINT)-1;										\
	}																\
	else															\
	{																\
		*(_Result) = 0;												\
	}																\
}

//
// This macro will compare network addresses.
//
//  A - Is a network address.
//
//  B - Is a network address.
//
//  Result - The result of comparing two network address.
//
//  Result != 0 Implies inequality.
//  Result == 0 Implies equality.
//
//
#define FDDI_COMPARE_NETWORK_ADDRESSES_EQ(_A, _B, _Length, _Result)	\
{																	\
	if ((*(USHORT UNALIGNED *)(_A) ==								\
			*(USHORT UNALIGNED *)(_B)) &&							\
		 (((_Length) == 2) ||										\
			(*(ULONG UNALIGNED *)((PUCHAR)(_A) + 2) ==				\
			 *(ULONG UNALIGNED *)((PUCHAR)(_B) + 2))))				\
	{																\
		*(_Result) = 0;												\
	}																\
	else															\
	{																\
		*(_Result) = 1;												\
	}																\
}


//
// This macro is used to copy from one network address to
// another.
//
#define FDDI_COPY_NETWORK_ADDRESS(D, S, AddressLength)				\
{																	\
	PCHAR _D = (D);                                                 \
	PCHAR _S = (S);                                                 \
	UINT _C = (AddressLength);                                      \
	for ( ; _C > 0 ; _D++, _S++, _C--)                              \
	{                                                               \
		*_D = *_S;                                                  \
	}																\
}


//
//UINT
//FDDI_QUERY_FILTER_CLASSES(
//	IN PFDDI_FILTER Filter
//	)
//
// This macro returns the currently enabled filter classes.
//
// NOTE: THIS MACRO ASSUMES THAT THE FILTER LOCK IS HELD.
//
#define FDDI_QUERY_FILTER_CLASSES(Filter) ((Filter)->CombinedPacketFilter)


//
//UINT
//FDDI_QUERY_PACKET_FILTER(
//	IN PFDDI_FILTER Filter,
//	IN NDIS_HANDLE NdisFilterHandle
//	)
//
// This macro returns the currently enabled filter classes for a specific
// open instance.
//
// NOTE: THIS MACRO ASSUMES THAT THE FILTER LOCK IS HELD.
//
#define FDDI_QUERY_PACKET_FILTER(Filter, NdisFilterHandle) \
		(((PFDDI_BINDING_INFO)(NdisFilterHandle))->PacketFilters)


//
//UINT
//FDDI_NUMBER_OF_GLOBAL_FILTER_LONG_ADDRESSES(
//	IN PFDDI_FILTER Filter
//	)
//
// This macro returns the number of multicast addresses in the
// multicast address list.
//
// NOTE: THIS MACRO ASSUMES THAT THE FILTER LOCK IS HELD.
//
#define FDDI_NUMBER_OF_GLOBAL_FILTER_LONG_ADDRESSES(Filter) ((Filter)->NumberOfLongAddresses)


//
//UINT
//FDDI_NUMBER_OF_GLOBAL_FILTER_SHORT_ADDRESSES(
//	IN PFDDI_FILTER Filter
//	)
//
// This macro returns the number of multicast addresses in the
// multicast address list.
//
// NOTE: THIS MACRO ASSUMES THAT THE FILTER LOCK IS HELD.
//
#define FDDI_NUMBER_OF_GLOBAL_FILTER_SHORT_ADDRESSES(Filter) ((Filter)->NumberOfShortAddresses)


//
// An action routine type.	The routines are called
// when a filter type is set for the first time or
// no more bindings require a particular type of filter.
//
// NOTE: THIS ROUTINE SHOULD ASSUME THAT THE LOCK IS ACQUIRED.
//
typedef
NDIS_STATUS
(*FDDI_FILTER_CHANGE)(
	IN UINT OldFilterClasses,
	IN UINT NewFilterClasses,
	IN NDIS_HANDLE MacBindingHandle,
	IN PNDIS_REQUEST NdisRequest,
	IN BOOLEAN Set
	);

//
// This action routine is called when a new multicast address
// list is given to the filter. The action routine is given
// arrays containing the old and new multicast addresses.
//
// NOTE: THIS ROUTINE SHOULD ASSUME THAT THE LOCK IS ACQUIRED.
//
typedef
NDIS_STATUS
(*FDDI_ADDRESS_CHANGE)(
	IN UINT OldLongAddressCount,
	IN CHAR OldLongAddresses[][FDDI_LENGTH_OF_LONG_ADDRESS],
	IN UINT NewLongAddressCount,
	IN CHAR NewLongAddresses[][FDDI_LENGTH_OF_LONG_ADDRESS],
	IN UINT OldShortAddressCount,
	IN CHAR OldShortAddresses[][FDDI_LENGTH_OF_SHORT_ADDRESS],
	IN UINT NewShortAddressCount,
	IN CHAR NewShortAddresses[][FDDI_LENGTH_OF_SHORT_ADDRESS],
	IN NDIS_HANDLE MacBindingHandle,
	IN PNDIS_REQUEST NdisRequest,
	IN BOOLEAN Set
	);

//
// This action routine is called when the mac requests a close for
// a particular binding *WHILE THE BINDING IS BEING INDICATED TO
// THE PROTOCOL*.  The filtering package can't get rid of the open
// right away.  So this routine will be called as soon as the
// NdisIndicateReceive returns.
//
// NOTE: THIS ROUTINE SHOULD ASSUME THAT THE LOCK IS ACQUIRED.
//
typedef
VOID
(*FDDI_DEFERRED_CLOSE)(
	IN NDIS_HANDLE MacBindingHandle
	);

typedef ULONG FDDI_MASK,*PFDDI_MASK;

//
// Maximum number of opens the filter package will support.  This is
// the max so that bit masks can be used instead of a spaghetti of
// pointers.
//
#define FDDI_FILTER_MAX_OPENS (sizeof(ULONG) * 8)

//
// The binding info is threaded on two lists.  When
// the binding is free it is on a single freelist.
//
// When the binding is being used it is on an index list
// and possibly on seperate broadcast and directed lists.
//
typedef struct _FDDI_BINDING_INFO
{
	NDIS_HANDLE MacBindingHandle;
	NDIS_HANDLE NdisBindingContext;
	UINT PacketFilters;
	ULONG References;
	BOOLEAN ReceivedAPacket;
	UCHAR FilterIndex;

	struct _FDDI_BINDING_INFO *NextOpen;
	//
	//  The following pointers are used to travers the specific
	//  filter lists.
	//
	UINT						OldPacketFilters;
	struct _FDDI_BINDING_INFO	*NextDirected;
	struct _FDDI_BINDING_INFO	*NextBMS;

} FDDI_BINDING_INFO,*PFDDI_BINDING_INFO;

//
// An opaque type that contains a filter database.
// The MAC need not know how it is structured.
//
typedef struct _FDDI_FILTER
{

	//
	// Spin lock used to protect the filter from multiple accesses.
	//
	PNDIS_SPIN_LOCK Lock;

	//
	// Pointer to an array of 6 character arrays holding the
	// multicast addresses requested for filtering.
	//
	CHAR (*MulticastLongAddresses)[FDDI_LENGTH_OF_LONG_ADDRESS];

	//
	// Pointer to an array of FDDI_MASKS that work in conjuction with
	// the MulticastLongAddress array.  In the masks, a bit being enabled
	// indicates that the binding with the given FilterIndex is using
	// the corresponding address.
	//
	FDDI_MASK *BindingsUsingLongAddress;

	//
	// Pointer to an array of 2 character arrays holding the
	// multicast addresses requested for filtering.
	//
	CHAR (*MulticastShortAddresses)[FDDI_LENGTH_OF_SHORT_ADDRESS];

	//
	// Pointer to an array of FDDI_MASKS that work in conjuction with
	// the MulticastShortAddress array.  In the masks, a bit being enabled
	// indicates that the binding with the given FilterIndex is using
	// the corresponding address.
	//
	FDDI_MASK *BindingsUsingShortAddress;

	//
	// Combination of all the filters of all the open bindings.
	//
	UINT CombinedPacketFilter;

	//
	// Pointer for traversing the open list.
	//
	PFDDI_BINDING_INFO OpenList;


	//
	// Action routines to be invoked on notable changes in the filter.
	//

	FDDI_ADDRESS_CHANGE AddressChangeAction;
	FDDI_FILTER_CHANGE FilterChangeAction;
	FDDI_DEFERRED_CLOSE CloseAction;

	//
	// The maximum number of long addresses used for filtering.
	//
	UINT MaximumMulticastLongAddresses;

	//
	// The maximum number of short addresses used for filtering.
	//
	UINT MaximumMulticastShortAddresses;

	//
	// The current number of addresses in the LongAddress filter.
	//
	UINT NumberOfLongAddresses;

	//
	// The current number of addresses in the ShortAddress filter.
	//
	UINT NumberOfShortAddresses;

	//
	// Bit mask of opens that are available.
	//
	ULONG FreeBindingMask;

	//
	// Long Address of the adapter.
	//
	UCHAR AdapterLongAddress[FDDI_LENGTH_OF_LONG_ADDRESS];

	//
	// Short Address of the adapter.
	//
	UCHAR AdapterShortAddress[FDDI_LENGTH_OF_SHORT_ADDRESS];

	//
	//  Filter specific list for optimization.
	//
	UINT				OldNumberOfLongAddresses;
	UINT				OldNumberOfShortAddresses;
	CHAR				(*OldMulticastLongAddresses)[FDDI_LENGTH_OF_LONG_ADDRESS];
	CHAR				(*OldMulticastShortAddresses)[FDDI_LENGTH_OF_SHORT_ADDRESS];
	FDDI_MASK			*OldBindingsUsingLongAddress;
	FDDI_MASK			*OldBindingsUsingShortAddress;
	UINT				OldCombinedPacketFilter;
	//
	// The list of bindings are seperated for directed and broadcast/multicast
	// Promiscuous bindings are on both lists
	//
	PFDDI_BINDING_INFO	DirectedList;	// List of bindings for directed packets
	PFDDI_BINDING_INFO	BMSList;		// List of bindings for broadcast/multicast/SMT packets

	struct _NDIS_MINIPORT_BLOCK *Miniport;
} FDDI_FILTER, *PFDDI_FILTER;

//
// Only for internal wrapper use.
//
VOID
FddiInitializePackage(
	VOID
	);

VOID
FddiReferencePackage(
	VOID
	);

VOID
FddiDereferencePackage(
	VOID
	);

//
// Exported routines
//

EXPORT
BOOLEAN
FddiCreateFilter(
	IN UINT MaximumMulticastLongAddresses,
	IN UINT MaximumMulticastShortAddresses,
	IN FDDI_ADDRESS_CHANGE AddressChangeAction,
	IN FDDI_FILTER_CHANGE FilterChangeAction,
	IN FDDI_DEFERRED_CLOSE CloseAction,
	IN PUCHAR AdapterLongAddress,
	IN PUCHAR AdapterShortAddress,
	IN PNDIS_SPIN_LOCK Lock,
	OUT PFDDI_FILTER *Filter
	);

EXPORT
VOID
FddiDeleteFilter(
	IN PFDDI_FILTER Filter
	);

EXPORT
BOOLEAN
FddiNoteFilterOpenAdapter(
	IN PFDDI_FILTER Filter,
	IN NDIS_HANDLE MacBindingHandle,
	IN NDIS_HANDLE NdisBindingContext,
	OUT PNDIS_HANDLE NdisFilterHandle
	);

EXPORT
NDIS_STATUS
FddiDeleteFilterOpenAdapter(
	IN PFDDI_FILTER Filter,
	IN NDIS_HANDLE NdisFilterHandle,
	IN PNDIS_REQUEST NdisRequest
	);

EXPORT
NDIS_STATUS
FddiChangeFilterLongAddresses(
	IN PFDDI_FILTER Filter,
	IN NDIS_HANDLE NdisFilterHandle,
	IN PNDIS_REQUEST NdisRequest,
	IN UINT AddressCount,
	IN CHAR Addresses[][FDDI_LENGTH_OF_LONG_ADDRESS],
	IN BOOLEAN Set
	);

EXPORT
NDIS_STATUS
FddiChangeFilterShortAddresses(
	IN PFDDI_FILTER Filter,
	IN NDIS_HANDLE NdisFilterHandle,
	IN PNDIS_REQUEST NdisRequest,
	IN UINT AddressCount,
	IN CHAR Addresses[][FDDI_LENGTH_OF_SHORT_ADDRESS],
	IN BOOLEAN Set
	);


#define	FddiShouldAddressLoopBackMacro( _Filter,												\
										_Address,												\
										_AddressLength,											\
										_pfLoopBack,											\
										_pfSelfDirected)										\
{																								\
	INT ResultOfAddressCheck;																	\
																								\
	UINT CombinedFilters, i;																	\
																								\
	CombinedFilters = FDDI_QUERY_FILTER_CLASSES(_Filter);										\
																								\
	*(_pfLoopBack) = FALSE;																		\
	*(_pfSelfDirected) = FALSE;																	\
																								\
	do																							\
	{																							\
		/*																						\
		 * Check if it *at least* has the multicast address bit.								\
		 */																						\
																								\
		FDDI_IS_MULTICAST(_Address,																\
						  (_AddressLength),														\
						  &ResultOfAddressCheck);												\
																								\
		if (ResultOfAddressCheck)																\
		{																						\
			/*																					\
			 * It is at least a multicast address.  Check to see if								\
			 * it is a broadcast address.														\
			 */																					\
																								\
			FDDI_IS_BROADCAST(_Address,															\
							  (_AddressLength),													\
							  &ResultOfAddressCheck);											\
																								\
			if (ResultOfAddressCheck)															\
			{																					\
				if (CombinedFilters & NDIS_PACKET_TYPE_BROADCAST)								\
				{																				\
					*(_pfLoopBack) = TRUE;														\
					break;																		\
				}																				\
			}																					\
			else																				\
			{																					\
				if ((CombinedFilters & NDIS_PACKET_TYPE_ALL_MULTICAST) ||						\
					((CombinedFilters & NDIS_PACKET_TYPE_MULTICAST) &&							\
					 ((((_AddressLength) == FDDI_LENGTH_OF_LONG_ADDRESS) &&		 				\
						FddiFindMulticastLongAddress((_Filter)->NumberOfLongAddresses,			\
													(_Filter)->MulticastLongAddresses,			\
													_Address,									\
													&i)) ||						 				\
					  (((_AddressLength) == FDDI_LENGTH_OF_SHORT_ADDRESS) &&					\
						FddiFindMulticastShortAddress((_Filter)->NumberOfShortAddresses, 		\
													  (_Filter)->MulticastShortAddresses,		\
													  _Address,									\
													  &i)))))									\
				{																				\
					*(_pfLoopBack) = TRUE;														\
					break;																		\
				}																				\
			}																					\
		}																						\
		else																					\
		{																						\
			/*																					\
			 * Directed to ourself?																\
			 */																					\
			if ((_AddressLength) == FDDI_LENGTH_OF_LONG_ADDRESS)								\
			{																					\
				FDDI_COMPARE_NETWORK_ADDRESSES_EQ((_Filter)->AdapterLongAddress,				\
												_Address,										\
												FDDI_LENGTH_OF_LONG_ADDRESS,					\
												&ResultOfAddressCheck							\
												);												\
			}																					\
			else																				\
			{																					\
				FDDI_COMPARE_NETWORK_ADDRESSES_EQ((_Filter)->AdapterShortAddress,				\
												_Address,										\
												FDDI_LENGTH_OF_SHORT_ADDRESS,					\
												&ResultOfAddressCheck							\
												);												\
			}																					\
																								\
			if (ResultOfAddressCheck == 0)														\
			{																					\
				*(_pfLoopBack) = TRUE;															\
				*(_pfSelfDirected) = TRUE;														\
				break;																			\
			}																					\
		}																						\
	} while (FALSE);																			\
																								\
	/*																							\
	 * First check if the filter is promiscuous.												\
	 */																							\
																								\
	if (CombinedFilters & (NDIS_PACKET_TYPE_PROMISCUOUS | NDIS_PACKET_TYPE_ALL_LOCAL))			\
	{																							\
		*(_pfLoopBack) = TRUE;																	\
	}																							\
}

EXPORT
BOOLEAN
FddiShouldAddressLoopBack(
	IN PFDDI_FILTER Filter,
	IN CHAR Address[],
	IN UINT LengthOfAddress
	);

EXPORT
NDIS_STATUS
FddiFilterAdjust(
	IN PFDDI_FILTER Filter,
	IN NDIS_HANDLE NdisFilterHandle,
	IN PNDIS_REQUEST NdisRequest,
	IN UINT FilterClasses,
	IN BOOLEAN Set
	);

EXPORT
UINT
FddiNumberOfOpenFilterLongAddresses(
	IN PFDDI_FILTER Filter,
	IN NDIS_HANDLE NdisFilterHandle
	);

EXPORT
UINT
FddiNumberOfOpenFilterShortAddresses(
	IN PFDDI_FILTER Filter,
	IN NDIS_HANDLE NdisFilterHandle
	);

EXPORT
VOID
FddiQueryGlobalFilterLongAddresses(
	OUT PNDIS_STATUS Status,
	IN PFDDI_FILTER Filter,
	IN UINT SizeOfArray,
	OUT PUINT NumberOfAddresses,
	IN OUT CHAR AddressArray[][FDDI_LENGTH_OF_LONG_ADDRESS]
	);

EXPORT
VOID
FddiQueryGlobalFilterShortAddresses(
	OUT PNDIS_STATUS Status,
	IN PFDDI_FILTER Filter,
	IN UINT SizeOfArray,
	OUT PUINT NumberOfAddresses,
	IN OUT CHAR AddressArray[][FDDI_LENGTH_OF_SHORT_ADDRESS]
	);

EXPORT
VOID
FddiQueryOpenFilterLongAddresses(
	OUT PNDIS_STATUS Status,
	IN PFDDI_FILTER Filter,
	IN NDIS_HANDLE NdisFilterHandle,
	IN UINT SizeOfArray,
	OUT PUINT NumberOfAddresses,
	IN OUT CHAR AddressArray[][FDDI_LENGTH_OF_LONG_ADDRESS]
	);

EXPORT
VOID
FddiQueryOpenFilterShortAddresses(
	OUT PNDIS_STATUS Status,
	IN PFDDI_FILTER Filter,
	IN NDIS_HANDLE NdisFilterHandle,
	IN UINT SizeOfArray,
	OUT PUINT NumberOfAddresses,
	IN OUT CHAR AddressArray[][FDDI_LENGTH_OF_SHORT_ADDRESS]
	);

EXPORT
VOID
FddiFilterIndicateReceive(
	IN PFDDI_FILTER Filter,
	IN NDIS_HANDLE MacReceiveContext,
	IN PCHAR Address,
	IN UINT AddressLength,
	IN PVOID HeaderBuffer,
	IN UINT HeaderBufferSize,
	IN PVOID LookaheadBuffer,
	IN UINT LookaheadBufferSize,
	IN UINT PacketSize
	);

EXPORT
VOID
FddiFilterDprIndicateReceive(
	IN PFDDI_FILTER Filter,
	IN NDIS_HANDLE MacReceiveContext,
	IN PCHAR Address,
	IN UINT AddressLength,
	IN PVOID HeaderBuffer,
	IN UINT HeaderBufferSize,
	IN PVOID LookaheadBuffer,
	IN UINT LookaheadBufferSize,
	IN UINT PacketSize
	);

EXPORT
VOID
FddiFilterDprIndicateReceivePacket(
	IN NDIS_HANDLE	MiniportHandle,
	IN PPNDIS_PACKET ReceivedPackets,
	IN UINT NumberOfPackets
	);

EXPORT
VOID
FddiFilterIndicateReceiveComplete(
	IN PFDDI_FILTER Filter
	);

EXPORT
VOID
FddiFilterDprIndicateReceiveComplete(
	IN PFDDI_FILTER Filter
	);

#endif // _FDDI_FILTER_DEFS_

