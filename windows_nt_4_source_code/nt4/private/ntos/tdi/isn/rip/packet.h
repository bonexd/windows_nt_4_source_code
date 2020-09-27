/*******************************************************************/
/*	      Copyright(c)  1993 Microsoft Corporation		   */
/*******************************************************************/

//***
//
// Filename:	packet.h
//
// Description: Contains general definitions for the ipx and rip packets
//
// Author:	Stefan Solomon (stefans)    October 4, 1993.
//
// Revision History:
//
//***

#ifndef _PACKET_
#define _PACKET_

//*** Socket Numbers

#define IPX_RIP_SOCKET	    (USHORT)0x453
#define IPX_NETBIOS_SOCKET  (USHORT)0x455

//*** Packet Types

#define IPX_RIP_TYPE	    1	   // RIP request/reply packet
#define IPX_NETBIOS_TYPE    20	   // Netbios propagated packet

//*** RIP Operations

#define RIP_REQUEST	   (USHORT)1
#define RIP_RESPONSE	   (USHORT)2

//*** Offsets into the IPX header

#define IPXH_HDRSIZE	    30	    // Size of the IPX header
#define IPXH_CHECKSUM	    0	    // Checksum
#define IPXH_LENGTH	    2	    // Length
#define IPXH_XPORTCTL	    4	    // Transport Control
#define IPXH_PKTTYPE	    5	    // Packet Type
#define IPXH_DESTADDR	    6	    // Dest. Address (Total)
#define IPXH_DESTNET	    6	    // Dest. Network Address
#define IPXH_DESTNODE	    10	    // Dest. Node Address
#define IPXH_DESTSOCK	    16	    // Dest. Socket Number
#define IPXH_SRCADDR	    18	    // Source Address (Total)
#define IPXH_SRCNET	    18	    // Source Network Address
#define IPXH_SRCNODE	    22	    // Source Node Address
#define IPXH_SRCSOCK	    28	    // Source Socket Number

#define IPX_NET_LEN	    4
#define IPX_NODE_LEN	    6

//*** RIP OPERATION FIELD

#define RIP_OPCODE	    30	 // rip operation code offset

//*** Network entry structure in the RIP request/response

#define RIP_INFO	    32 // first network entry in the rip packet

#define NE_ENTRYSIZE	    8	    // 4 network + 2 hops + 2 ticks
#define NE_NETNUMBER	    0	    // network number offset
#define NE_NROFHOPS	    4	    // number of hops offset
#define NE_NROFTICKS	    6	    // nymber of ticks offset

//*** maximum nr of hops for a normal packet ***

#define IPX_MAX_HOPS	    16

//*** define rip response packet size

#define RIP_RESPONSE_PACKET_LEN 	432

//*** offsets into the netbios session data packet ***

#define NB_CONNECTION_CONTROL_FLAG	30
#define NB_DATA_STREAM_TYPE		31
#define NB_TOTAL_DATA_LENGTH		38

#endif
