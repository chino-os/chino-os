// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <netinet/in.h>
#include <sys/socket.h>

#define IPVERSION 4 /* IP version number */
#define IPDEFTTL 64 /* default ttl, from RFC 1340 */

struct iphdr {
#if _BYTE_ORDER == _LITTLE_ENDIAN
    unsigned int ihl : 4;
    unsigned int version : 4;
#elif _BYTE_ORDER == _BIG_ENDIAN
    unsigned int version : 4;
    unsigned int ihl : 4;
#else
#error "unknown endian"
#endif
    uint8_t tos;
    uint16_t tot_len;
    uint16_t id;
    uint16_t frag_off;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t check;
    uint32_t saddr;
    uint32_t daddr;

    /* The options start here. */
} __attribute__((packed));
