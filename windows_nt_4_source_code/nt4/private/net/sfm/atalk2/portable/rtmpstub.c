/*   RtmpStub.c,  /appletalk/source,  Garth Conboy,  10/07/88  */
/*   Copyright (c) 1988 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.
     GC - (12/10/89): AppleTalk phase II comes to town; we're now just
                      an RTMP stub.
     GC - (08/18/90): New error logging mechanism.
     GC - (01/20/92): Removed usage of numberOfConfiguredPorts; portDescriptors
                      may now be sparse, we use portActive instead.
     GC - (03/23/92): Try a little harder to figure out our port's zone name
                      when a router comes up on a cable, after startup or after
                      all routers have aged out.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     Handle the RTMP protocol for a non-routing node.

*/

#define IncludeRtmpStubErrors 1

#include "atalk.h"

ExternForVisibleFunction TimerHandler RtmpAgingTimerExpired;

static Boolean firstCall = True;

void far ShutdownRtmpStub(void)
{

  firstCall = True;

}  /* ShutdownRtmpStub */

long far RtmpPacketIn(AppleTalkErrorCode errorCode,
                      long unsigned userData,
                      int port,
                      AppleTalkAddress source,
                      long destinationSocket,
                      int protocolType,
                      char far *datagram,
                      int datagramLength,
                      AppleTalkAddress actualDestination)
{
  ExtendedAppleTalkNodeNumber sendersNode, ourNode;
  AppleTalkNetworkRange cableRange;
  AppleTalkAddress ourAddress;

  /* "Use" unused formals. */

  userData, source, actualDestination;

  /* Do we care? */

  if (errorCode is ATsocketClosed)
     return((long)True);
  else if (errorCode isnt ATnoError)
  {
     ErrorLog("RtmpPacketIn", ISevError, __LINE__, port,
              IErrRtmpStubBadIncomingError, IMsgRtmpStubBadIncomingError,
              Insert0());
     return((long)True);
  }
  if (protocolType isnt DdpProtocolRtmpResponseOrData)
     return((long)True);

  if (firstCall)
  {
     StartTimer(RtmpAgingTimerExpired, RtmpAgingTimerSeconds, 0, empty);
     firstCall = False;
  }

  /* Non-routing half port?  Shouldn't really ever happen, except at startup,
     and we don't care then. */

  if (portDescriptors[port].portType is NonAppleTalkHalfPort)
     return((long)True);

  /* Is the packet long enough to have any interestng data? */

  if (portDescriptors[port].extendedNetwork and
      datagramLength < RtmpRangeEndOffset + 2)
  {
     ErrorLog("RtmpPacketIn", ISevWarning, __LINE__, port,
              IErrRtmpStubTooShortExt, IMsgRtmpStubTooShortExt,
              Insert0());
     return((long)True);
  }
  else if (not portDescriptors[port].extendedNetwork and
           datagramLength < RtmpSendersIdOffset)
  {
     ErrorLog("RtmpPacketIn", ISevWarning, __LINE__, port,
              IErrRtmpStubTooShort, IMsgRtmpStubTooShort,
              Insert0());
     return((long)True);
  }

  /* Okay, pull out what we're interested in. */

  sendersNode.networkNumber =
         (short unsigned)(datagram[RtmpSendersNetworkOffset] << 8);
  sendersNode.networkNumber +=
         (unsigned char)datagram[RtmpSendersNetworkOffset + 1];
  if (datagram[RtmpSendersIdLengthOffset] isnt 8)
  {
     ErrorLog("RtmpPacketIn", ISevWarning, __LINE__, port,
              IErrRtmpStubBadIdLength, IMsgRtmpStubBadIdLength,
              Insert0());
     return((long)True);
  }
  sendersNode.nodeNumber = datagram[RtmpSendersIdOffset];
  if (portDescriptors[port].extendedNetwork)
  {
     cableRange.firstNetworkNumber =
         (short unsigned)(datagram[RtmpRangeStartOffset] << 8);
     cableRange.firstNetworkNumber +=
         (unsigned char )datagram[RtmpRangeStartOffset + 1];
     cableRange.lastNetworkNumber =
         (short unsigned)(datagram[RtmpRangeEndOffset] << 8);
     cableRange.lastNetworkNumber +=
         (unsigned char )datagram[RtmpRangeEndOffset + 1];
     if (not CheckNetworkRange(cableRange))
        return((long)True);
  }

  /* On a non-extended network, we don't have to do any checking, just copy
     the information into A-ROUTER and THIS-NET. */

  if (not portDescriptors[port].extendedNetwork)
  {
     portDescriptors[port].seenRouterRecently = True;
     portDescriptors[port].lastRouterTime = CurrentRelativeTime();
     portDescriptors[port].aRouter = sendersNode;
     if (portDescriptors[port].thisCableRange.firstNetworkNumber is
         UnknownNetworkNumber)
        portDescriptors[port].activeNodes->extendedNode.networkNumber =
            portDescriptors[port].thisCableRange.firstNetworkNumber =
                 sendersNode.networkNumber;
     else if (portDescriptors[port].thisCableRange.firstNetworkNumber isnt
              sendersNode.networkNumber)
        ErrorLog("RtmpPacketIn", ISevWarning, __LINE__, port,
                 IErrRtmpStubNetNumChanged, IMsgRtmpStubNetNumChanged,
                 Insert0());
     return((long)True);
  }

  /* On extended networks, we may want to reject the information: If we
     already know about a router, the cable ranges must exacly match; If we
     don't know about a router, our node's network number must be within
     the cable range specified by the first tuple.  The latter test will
     discard the information if our node is in the startup range (which is
     the right thing to do). */

  if (portDescriptors[port].seenRouterRecently)
     if (cableRange.firstNetworkNumber isnt
              portDescriptors[port].thisCableRange.firstNetworkNumber or
         cableRange.lastNetworkNumber isnt
              portDescriptors[port].thisCableRange.lastNetworkNumber)
        return((long)True);
     else
        /* Okay */ ;

  /* Okay, we've seen a valid Rtmp data, this should allow us to find the
     zone name for the port.  We do this outside of the "not seenRouterRecently"
     case because the first time a router send out an Rtmp data it may not
     know everything yet, or GetNetworkInfoForNode() (which calls WaitFor) may
     really do a hard wait and we may need to try it a second time (due to not
     repsonding to Aarp LocateNode's the first time through... the second time
     our addresses should be cached by the remote router and he won't need
     to do a LocateNode again). */

  if (not portDescriptors[port].thisZoneValid)
  {
     if (MapSocketToAddress(destinationSocket, &ourAddress) isnt ATnoError)
     {
        ErrorLog("RtmpPacketIn", ISevError, __LINE__, port,
                 IErrRtmpStubCouldntMapAddr, IMsgRtmpStubCouldntMapAddr,
                 Insert0());
        return((long)True);
     }
     if (not IsWithinNetworkRange(ourAddress.networkNumber, &cableRange))
        return((long)True);

     /* Oh boy, a new router, see if it will tell us our zone name. */

     ourNode.networkNumber = ourAddress.networkNumber;
     ourNode.nodeNumber = ourAddress.nodeNumber;
     GetNetworkInfoForNode(port, ourNode, False);
  }

  /* Well, it looks like we win.  Copy the information.  Don't do it
     if we're a router [maybe a proxy node on arouting port] -- we don't
     want "aRouter" to shift away from "us." */

  if (not portDescriptors[port].routerRunning)
  {
     portDescriptors[port].seenRouterRecently = True;
     portDescriptors[port].lastRouterTime = CurrentRelativeTime();
     portDescriptors[port].aRouter = sendersNode;
     portDescriptors[port].thisCableRange = cableRange;
  }

  /* All set! */

  return((long)True);

}  /* RtmpPacketIn */

ExternForVisibleFunction void far
         RtmpAgingTimerExpired(long unsigned timerId,
                               int additionalDataSize,
                               char far *additionalData)
{
  int port;
  long unsigned now = CurrentRelativeTime();

  /* "Use" unused formals. */

  timerId, additionalData, additionalDataSize;

  /* Walk through our active ports; have any routers aged out? */

  for (port = 0; port < MaximumNumberOfPorts; port += 1)
     if (portDescriptors[port].portActive and
         not portDescriptors[port].routerRunning and
         portDescriptors[port].seenRouterRecently and
         (portDescriptors[port].lastRouterTime +
          RtmpAgingTimerSeconds) < now)
     {

        /* Age out A-ROUTER; on extended networks age out THIS-CABLE-RANGE
           and THIS-ZONE too. */

        portDescriptors[port].seenRouterRecently = False;
        if (portDescriptors[port].extendedNetwork)
        {
           PortHandlers portHandlers;
           portHandlers = &portSpecificInfo[portDescriptors[port].portType];

           portDescriptors[port].thisZoneValid = False;
           portDescriptors[port].thisCableRange.firstNetworkNumber =
                 FirstValidNetworkNumber;
           portDescriptors[port].thisCableRange.lastNetworkNumber =
                 LastStartupNetworkNumber;

           /* If we have a zone-multicast address that is not broadcast,
              age it out too. */

           if (not FixedCompareCaseSensitive(portDescriptors[port].
                                                 zoneMulticastAddress,
                                             portHandlers->
                                                 hardwareAddressLength,
                                             portHandlers->broadcastAddress,
                                             portHandlers->
                                                 hardwareAddressLength))
              (*portHandlers->removeMulticastAddress)(port, 1,
                                                      portDescriptors[port].
                                                          zoneMulticastAddress);
           FillMem(portDescriptors[port].zoneMulticastAddress,
                   0, portHandlers->hardwareAddressLength);
        }
     }

  /* Restart the timer and flee. */

  StartTimer(RtmpAgingTimerExpired, RtmpAgingTimerSeconds, 0, empty);
  HandleDeferredTimerChecks();
  return;

}  /* RtmpAgingTimerExpired */
