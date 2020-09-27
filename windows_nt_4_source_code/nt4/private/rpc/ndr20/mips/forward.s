//      TITLE("NdrProxyForwardingFunctions")
//++
//
//  Copyright (c) 1994  Microsoft Corporation.  All rights reserved.
//
//  Module Name:
//     forward.s
//
//  Abstract:
//     This module implements the proxy forwarding functions.
//
//  Author:
//    ShannonC    25-Oct-94
//
//  Environment:
//     Any mode.
//
//  Revision History:
//
//--

#include "ksmips.h"

//++
//
// VOID STDMETHODCALLTYPE
// NdrProxyForwardingFunction(
//    IN IUnknown *This,
//    ...
//    )
//
// Routine Description:
//
//    This function forwards a call to the proxy for the base interface.
//
// Arguments:
//
//    This (a0) - Points to an interface proxy.
//
// Return Value:
//
//    None.
//
//--

#define SUBCLASS_OFFSET     16

#define DELEGATION_FORWARDER(Method)        \
    ALTERNATE_ENTRY(NdrProxyForwardingFunction##Method)      \
                                                        \
    addiu    t0, zero, Method * 4;                           \
    j       NdrProxyForwardingFunction;


//On entry, t0 contains the vtbl offset of the method.
//          a0 contains the this pointer.
LEAF_ENTRY( NdrProxyForwardingFunction )
    lw      a0, SUBCLASS_OFFSET(a0); //Get this pointer for base interface.
    lw      t1,(a0);                 //Get lpVtbl
    addu     t0, t0, t1;
    lw      t0, (t0);
    j       t0;

    DELEGATION_FORWARDER(3)
    DELEGATION_FORWARDER(4)
    DELEGATION_FORWARDER(5)
    DELEGATION_FORWARDER(6)
    DELEGATION_FORWARDER(7)
    DELEGATION_FORWARDER(8)
    DELEGATION_FORWARDER(9)
    DELEGATION_FORWARDER(10)
    DELEGATION_FORWARDER(11)
    DELEGATION_FORWARDER(12)
    DELEGATION_FORWARDER(13)
    DELEGATION_FORWARDER(14)
    DELEGATION_FORWARDER(15)
    DELEGATION_FORWARDER(16)
    DELEGATION_FORWARDER(17)
    DELEGATION_FORWARDER(18)
    DELEGATION_FORWARDER(19)
    DELEGATION_FORWARDER(20)
    DELEGATION_FORWARDER(21)
    DELEGATION_FORWARDER(22)
    DELEGATION_FORWARDER(23)
    DELEGATION_FORWARDER(24)
    DELEGATION_FORWARDER(25)
    DELEGATION_FORWARDER(26)
    DELEGATION_FORWARDER(27)
    DELEGATION_FORWARDER(28)
    DELEGATION_FORWARDER(29)
    DELEGATION_FORWARDER(30)
    DELEGATION_FORWARDER(31)
    DELEGATION_FORWARDER(32)
    DELEGATION_FORWARDER(33)
    DELEGATION_FORWARDER(34)
    DELEGATION_FORWARDER(35)
    DELEGATION_FORWARDER(36)
    DELEGATION_FORWARDER(37)
    DELEGATION_FORWARDER(38)
    DELEGATION_FORWARDER(39)
    DELEGATION_FORWARDER(40)
    DELEGATION_FORWARDER(41)
    DELEGATION_FORWARDER(42)
    DELEGATION_FORWARDER(43)
    DELEGATION_FORWARDER(44)
    DELEGATION_FORWARDER(45)
    DELEGATION_FORWARDER(46)
    DELEGATION_FORWARDER(47)
    DELEGATION_FORWARDER(48)
    DELEGATION_FORWARDER(49)
    DELEGATION_FORWARDER(50)
    DELEGATION_FORWARDER(51)
    DELEGATION_FORWARDER(52)
    DELEGATION_FORWARDER(53)
    DELEGATION_FORWARDER(54)
    DELEGATION_FORWARDER(55)
    DELEGATION_FORWARDER(56)
    DELEGATION_FORWARDER(57)
    DELEGATION_FORWARDER(58)
    DELEGATION_FORWARDER(59)
    DELEGATION_FORWARDER(60)
    DELEGATION_FORWARDER(61)
    DELEGATION_FORWARDER(62)
    DELEGATION_FORWARDER(63)
    DELEGATION_FORWARDER(64)
    DELEGATION_FORWARDER(65)
    DELEGATION_FORWARDER(66)
    DELEGATION_FORWARDER(67)
    DELEGATION_FORWARDER(68)
    DELEGATION_FORWARDER(69)
    DELEGATION_FORWARDER(70)
    DELEGATION_FORWARDER(71)
    DELEGATION_FORWARDER(72)
    DELEGATION_FORWARDER(73)
    DELEGATION_FORWARDER(74)
    DELEGATION_FORWARDER(75)
    DELEGATION_FORWARDER(76)
    DELEGATION_FORWARDER(77)
    DELEGATION_FORWARDER(78)
    DELEGATION_FORWARDER(79)
    DELEGATION_FORWARDER(80)
    DELEGATION_FORWARDER(81)
    DELEGATION_FORWARDER(82)
    DELEGATION_FORWARDER(83)
    DELEGATION_FORWARDER(84)
    DELEGATION_FORWARDER(85)
    DELEGATION_FORWARDER(86)
    DELEGATION_FORWARDER(87)
    DELEGATION_FORWARDER(88)
    DELEGATION_FORWARDER(89)
    DELEGATION_FORWARDER(90)
    DELEGATION_FORWARDER(91)
    DELEGATION_FORWARDER(92)
    DELEGATION_FORWARDER(93)
    DELEGATION_FORWARDER(94)
    DELEGATION_FORWARDER(95)
    DELEGATION_FORWARDER(96)
    DELEGATION_FORWARDER(97)
    DELEGATION_FORWARDER(98)
    DELEGATION_FORWARDER(99)
    DELEGATION_FORWARDER(100)
    DELEGATION_FORWARDER(101)
    DELEGATION_FORWARDER(102)
    DELEGATION_FORWARDER(103)
    DELEGATION_FORWARDER(104)
    DELEGATION_FORWARDER(105)
    DELEGATION_FORWARDER(106)
    DELEGATION_FORWARDER(107)
    DELEGATION_FORWARDER(108)
    DELEGATION_FORWARDER(109)
    DELEGATION_FORWARDER(110)
    DELEGATION_FORWARDER(111)
    DELEGATION_FORWARDER(112)
    DELEGATION_FORWARDER(113)
    DELEGATION_FORWARDER(114)
    DELEGATION_FORWARDER(115)
    DELEGATION_FORWARDER(116)
    DELEGATION_FORWARDER(117)
    DELEGATION_FORWARDER(118)
    DELEGATION_FORWARDER(119)
    DELEGATION_FORWARDER(120)
    DELEGATION_FORWARDER(121)
    DELEGATION_FORWARDER(122)
    DELEGATION_FORWARDER(123)
    DELEGATION_FORWARDER(124)
    DELEGATION_FORWARDER(125)
    DELEGATION_FORWARDER(126)
    DELEGATION_FORWARDER(127)
    DELEGATION_FORWARDER(128)
    DELEGATION_FORWARDER(129)
    DELEGATION_FORWARDER(130)
    DELEGATION_FORWARDER(131)
    DELEGATION_FORWARDER(132)
    DELEGATION_FORWARDER(133)
    DELEGATION_FORWARDER(134)
    DELEGATION_FORWARDER(135)
    DELEGATION_FORWARDER(136)
    DELEGATION_FORWARDER(137)
    DELEGATION_FORWARDER(138)
    DELEGATION_FORWARDER(139)
    DELEGATION_FORWARDER(140)
    DELEGATION_FORWARDER(141)
    DELEGATION_FORWARDER(142)
    DELEGATION_FORWARDER(143)
    DELEGATION_FORWARDER(144)
    DELEGATION_FORWARDER(145)
    DELEGATION_FORWARDER(146)
    DELEGATION_FORWARDER(147)
    DELEGATION_FORWARDER(148)
    DELEGATION_FORWARDER(149)
    DELEGATION_FORWARDER(150)
    DELEGATION_FORWARDER(151)
    DELEGATION_FORWARDER(152)
    DELEGATION_FORWARDER(153)
    DELEGATION_FORWARDER(154)
    DELEGATION_FORWARDER(155)
    DELEGATION_FORWARDER(156)
    DELEGATION_FORWARDER(157)
    DELEGATION_FORWARDER(158)
    DELEGATION_FORWARDER(159)
    DELEGATION_FORWARDER(160)
    DELEGATION_FORWARDER(161)
    DELEGATION_FORWARDER(162)
    DELEGATION_FORWARDER(163)
    DELEGATION_FORWARDER(164)
    DELEGATION_FORWARDER(165)
    DELEGATION_FORWARDER(166)
    DELEGATION_FORWARDER(167)
    DELEGATION_FORWARDER(168)
    DELEGATION_FORWARDER(169)
    DELEGATION_FORWARDER(170)
    DELEGATION_FORWARDER(171)
    DELEGATION_FORWARDER(172)
    DELEGATION_FORWARDER(173)
    DELEGATION_FORWARDER(174)
    DELEGATION_FORWARDER(175)
    DELEGATION_FORWARDER(176)
    DELEGATION_FORWARDER(177)
    DELEGATION_FORWARDER(178)
    DELEGATION_FORWARDER(179)
    DELEGATION_FORWARDER(180)
    DELEGATION_FORWARDER(181)
    DELEGATION_FORWARDER(182)
    DELEGATION_FORWARDER(183)
    DELEGATION_FORWARDER(184)
    DELEGATION_FORWARDER(185)
    DELEGATION_FORWARDER(186)
    DELEGATION_FORWARDER(187)
    DELEGATION_FORWARDER(188)
    DELEGATION_FORWARDER(189)
    DELEGATION_FORWARDER(190)
    DELEGATION_FORWARDER(191)
    DELEGATION_FORWARDER(192)
    DELEGATION_FORWARDER(193)
    DELEGATION_FORWARDER(194)
    DELEGATION_FORWARDER(195)
    DELEGATION_FORWARDER(196)
    DELEGATION_FORWARDER(197)
    DELEGATION_FORWARDER(198)
    DELEGATION_FORWARDER(199)
    DELEGATION_FORWARDER(200)
    DELEGATION_FORWARDER(201)
    DELEGATION_FORWARDER(202)
    DELEGATION_FORWARDER(203)
    DELEGATION_FORWARDER(204)
    DELEGATION_FORWARDER(205)
    DELEGATION_FORWARDER(206)
    DELEGATION_FORWARDER(207)
    DELEGATION_FORWARDER(208)
    DELEGATION_FORWARDER(209)
    DELEGATION_FORWARDER(210)
    DELEGATION_FORWARDER(211)
    DELEGATION_FORWARDER(212)
    DELEGATION_FORWARDER(213)
    DELEGATION_FORWARDER(214)
    DELEGATION_FORWARDER(215)
    DELEGATION_FORWARDER(216)
    DELEGATION_FORWARDER(217)
    DELEGATION_FORWARDER(218)
    DELEGATION_FORWARDER(219)
    DELEGATION_FORWARDER(220)
    DELEGATION_FORWARDER(221)
    DELEGATION_FORWARDER(222)
    DELEGATION_FORWARDER(223)
    DELEGATION_FORWARDER(224)
    DELEGATION_FORWARDER(225)
    DELEGATION_FORWARDER(226)
    DELEGATION_FORWARDER(227)
    DELEGATION_FORWARDER(228)
    DELEGATION_FORWARDER(229)
    DELEGATION_FORWARDER(230)
    DELEGATION_FORWARDER(231)
    DELEGATION_FORWARDER(232)
    DELEGATION_FORWARDER(233)
    DELEGATION_FORWARDER(234)
    DELEGATION_FORWARDER(235)
    DELEGATION_FORWARDER(236)
    DELEGATION_FORWARDER(237)
    DELEGATION_FORWARDER(238)
    DELEGATION_FORWARDER(239)
    DELEGATION_FORWARDER(240)
    DELEGATION_FORWARDER(241)
    DELEGATION_FORWARDER(242)
    DELEGATION_FORWARDER(243)
    DELEGATION_FORWARDER(244)
    DELEGATION_FORWARDER(245)
    DELEGATION_FORWARDER(246)
    DELEGATION_FORWARDER(247)
    DELEGATION_FORWARDER(248)
    DELEGATION_FORWARDER(249)
    DELEGATION_FORWARDER(250)
    DELEGATION_FORWARDER(251)
    DELEGATION_FORWARDER(252)
    DELEGATION_FORWARDER(253)
    DELEGATION_FORWARDER(254)
    DELEGATION_FORWARDER(255)
    .end    NdrProxyForwardingFunction;
