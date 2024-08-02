// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <netinet/in.h>
#include <netinet/ip.h>

#define ICMP_ER 0   /* echo reply */
#define ICMP_DUR 3  /* destination unreachable */
#define ICMP_SQ 4   /* source quench */
#define ICMP_RD 5   /* redirect */
#define ICMP_ECHO 8 /* echo */
#define ICMP_TE 11  /* time exceeded */
#define ICMP_PP 12  /* parameter problem */
#define ICMP_TS 13  /* timestamp */
#define ICMP_TSR 14 /* timestamp reply */
#define ICMP_IRQ 15 /* information request */
#define ICMP_IR 16  /* information reply */
#define ICMP_AM 17  /* address mask request */
#define ICMP_AMR 18 /* address mask reply */

/** The standard ICMP header (unspecified 32 bit data) */
struct icmp_hdr {
    uint8_t type;
    uint8_t code;
    uint16_t chksum;
    uint32_t data;
} __attribute__((packed));

/** This is the standard ICMP header only that the u32_t data
 *  is split to two u16_t like ICMP echo needs it.
 */
struct icmp_echo_hdr {
    uint8_t type;
    uint8_t code;
    uint16_t chksum;
    uint16_t id;
    uint16_t seqno;
} __attribute__((packed));
