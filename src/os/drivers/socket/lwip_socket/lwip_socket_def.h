// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once

#define SOCKOPT_CHECK_OPTLEN(sock, optlen, opttype)                                                                    \
    if ((optlen) < sizeof(opttype))                                                                                    \
    return err(error_code::invalid_argument)

#define SO_SNDRCVTIMEO_OPTTYPE struct timeval
#define SO_SNDRCVTIMEO_SET(optval, val)                                                                                \
    do {                                                                                                               \
        u32_t loc = (val);                                                                                             \
        ((struct timeval *)(optval))->tv_sec = (long)((loc) / 1000U);                                                  \
        ((struct timeval *)(optval))->tv_usec = (long)(((loc) % 1000U) * 1000U);                                       \
    } while (0)
#define SO_SNDRCVTIMEO_GET_MS(optval)                                                                                  \
    ((((const struct timeval *)(optval))->tv_sec * 1000) + (((const struct timeval *)(optval))->tv_usec / 1000))

#define IS_SOCK_ADDR_LEN_VALID(namelen)                                                                                \
    (((namelen) == sizeof(struct sockaddr_in)) || ((namelen) == sizeof(struct sockaddr_in6)))
#define IS_SOCK_ADDR_TYPE_VALID(name) (((name)->sa_family == AF_INET) || ((name)->sa_family == AF_INET6))

#define IS_SOCK_ADDR_ALIGNED(name) ((((mem_ptr_t)(name)) % LWIP_MIN(4, MEM_ALIGNMENT)) == 0)

#define SOCKADDR_TO_IPADDR_PORT(sockaddr, ipaddr, port) sockaddr_to_ipaddr_port(sockaddr, ipaddr, &(port))

#define IP4ADDR_PORT_TO_SOCKADDR(sin, ipaddr, port)                                                                    \
    do {                                                                                                               \
        IP4ADDR_SOCKADDR_SET_LEN(sin);                                                                                 \
        (sin)->sin_family = AF_INET;                                                                                   \
        (sin)->sin_port = lwip_htons((port));                                                                          \
        inet_addr_from_ip4addr(&(sin)->sin_addr, ipaddr);                                                              \
        memset((sin)->sin_zero, 0, SIN_ZERO_LEN);                                                                      \
    } while (0)
#define SOCKADDR4_TO_IP4ADDR_PORT(sin, ipaddr, port)                                                                   \
    do {                                                                                                               \
        inet_addr_to_ip4addr(ip_2_ip4(ipaddr), &((sin)->sin_addr));                                                    \
        (port) = lwip_ntohs((sin)->sin_port);                                                                          \
    } while (0)

#define IP6ADDR_PORT_TO_SOCKADDR(sin6, ipaddr, port)                                                                   \
    do {                                                                                                               \
        IP6ADDR_SOCKADDR_SET_LEN(sin6);                                                                                \
        (sin6)->sin6_family = AF_INET6;                                                                                \
        (sin6)->sin6_port = lwip_htons((port));                                                                        \
        (sin6)->sin6_flowinfo = 0;                                                                                     \
        inet6_addr_from_ip6addr(&(sin6)->sin6_addr, ipaddr);                                                           \
        (sin6)->sin6_scope_id = ip6_addr_zone(ipaddr);                                                                 \
    } while (0)

#define SOCKADDR6_TO_IP6ADDR_PORT(sin6, ipaddr, port)                                                                  \
    do {                                                                                                               \
        inet6_addr_to_ip6addr(ip_2_ip6(ipaddr), &((sin6)->sin6_addr));                                                 \
        if (ip6_addr_has_scope(ip_2_ip6(ipaddr), IP6_UNKNOWN)) {                                                       \
            ip6_addr_set_zone(ip_2_ip6(ipaddr), (u8_t)((sin6)->sin6_scope_id));                                        \
        }                                                                                                              \
        (port) = lwip_ntohs((sin6)->sin6_port);                                                                        \
    } while (0)

#define IPADDR_PORT_TO_SOCKADDR(sockaddr, ipaddr, port)                                                                \
    do {                                                                                                               \
        if (IP_IS_ANY_TYPE_VAL(*ipaddr) || IP_IS_V6_VAL(*ipaddr)) {                                                    \
            IP6ADDR_PORT_TO_SOCKADDR((struct sockaddr_in6 *)(void *)(sockaddr), ip_2_ip6(ipaddr), port);               \
        } else {                                                                                                       \
            IP4ADDR_PORT_TO_SOCKADDR((struct sockaddr_in *)(void *)(sockaddr), ip_2_ip4(ipaddr), port);                \
        }                                                                                                              \
    } while (0)
