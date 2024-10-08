// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "lwip_socket.h"
#include "lwip_socket_def.h"
#include <chino/defer.h>
#include <chino/os/kernel/io.h>
#include <chino/os/kernel/ps.h>
#include <lwip/api.h>
#include <lwip/inet.h>
#include <lwip/ip.h>
#include <lwip/tcpip.h>
#include <netinet/in.h>
#include <posix/sockets.h>
#include <sys/uio.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::kernel::io;
using namespace chino::os::drivers;

namespace {
void lwip_tcpip_init_done(void *arg) {
    auto done_event = reinterpret_cast<os::event *>(arg);
    done_event->notify_one();
}

void sockaddr_to_ipaddr_port(const struct sockaddr *sockaddr, ip_addr_t *ipaddr, u16_t *port) {
    if ((sockaddr->sa_family) == AF_INET6) {
        SOCKADDR6_TO_IP6ADDR_PORT((const struct sockaddr_in6 *)(const void *)(sockaddr), ipaddr, *port);
        ipaddr->type = IPADDR_TYPE_V6;
    } else {
        SOCKADDR4_TO_IP4ADDR_PORT((const struct sockaddr_in *)(const void *)(sockaddr), ipaddr, *port);
        ipaddr->type = IPADDR_TYPE_V4;
    }
}

/** A struct sockaddr replacement that has the same alignment as sockaddr_in/
 *  sockaddr_in6 if instantiated.
 */
union sockaddr_aligned {
    struct sockaddr sa;
#if LWIP_IPV6
    struct sockaddr_in6 sin6;
#endif /* LWIP_IPV6 */
#if LWIP_IPV4
    struct sockaddr_in sin;
#endif /* LWIP_IPV4 */
};

union lwip_sock_lastdata {
    struct netbuf *netbuf;
    struct pbuf *pbuf;
};

struct lwip_sock {
    netconn *conn;
    lwip_sock_lastdata lastdata;

    lwip_sock(netconn *conn) noexcept : conn(conn) {}
};

struct tcpip_lock {
    tcpip_lock() noexcept { LOCK_TCPIP_CORE(); }
    ~tcpip_lock() { UNLOCK_TCPIP_CORE(); }
};
} // namespace

/* Convert a netbuf's address data to struct sockaddr */
static int lwip_sock_make_addr(struct netconn *conn, ip_addr_t *fromaddr, u16_t port, struct sockaddr *from,
                               socklen_t *fromlen) {
    int truncated = 0;
    union sockaddr_aligned saddr;

    LWIP_UNUSED_ARG(conn);

    LWIP_ASSERT("fromaddr != NULL", fromaddr != NULL);
    LWIP_ASSERT("from != NULL", from != NULL);
    LWIP_ASSERT("fromlen != NULL", fromlen != NULL);

#if LWIP_IPV4 && LWIP_IPV6
    /* Dual-stack: Map IPv4 addresses to IPv4 mapped IPv6 */
    if (NETCONNTYPE_ISIPV6(netconn_type(conn)) && IP_IS_V4(fromaddr)) {
        ip4_2_ipv4_mapped_ipv6(ip_2_ip6(fromaddr), ip_2_ip4(fromaddr));
        IP_SET_TYPE(fromaddr, IPADDR_TYPE_V6);
    }
#endif /* LWIP_IPV4 && LWIP_IPV6 */

    IPADDR_PORT_TO_SOCKADDR(&saddr, fromaddr, port);
    if (*fromlen < IPADDR_SOCKADDR_GET_LEN(&saddr)) {
        truncated = 1;
    } else if (*fromlen > IPADDR_SOCKADDR_GET_LEN(&saddr)) {
        *fromlen = IPADDR_SOCKADDR_GET_LEN(&saddr);
    }
    MEMCPY(from, &saddr, *fromlen);
    return truncated;
}

static err_t lwip_recvfrom_udp_raw(lwip_sock &sock, int flags, struct msghdr *msg, u16_t *datagram_len, int dbg_s) {
    struct netbuf *buf;
    u8_t apiflags;
    err_t err;
    u16_t buflen, copylen, copied;
    msg_iovlen_t i;

    LWIP_UNUSED_ARG(dbg_s);
    LWIP_ERROR("lwip_recvfrom_udp_raw: invalid arguments", (msg->msg_iov != NULL) || (msg->msg_iovlen <= 0),
               return ERR_ARG;);

    if (flags & MSG_DONTWAIT) {
        apiflags = NETCONN_DONTBLOCK;
    } else {
        apiflags = 0;
    }

    LWIP_DEBUGF(SOCKETS_DEBUG,
                ("lwip_recvfrom_udp_raw[UDP/RAW]: top sock.lastdata=%p\n", (void *)sock.lastdata.netbuf));
    /* Check if there is data left from the last recv operation. */
    buf = sock.lastdata.netbuf;
    if (buf == NULL) {
        /* No data was left from the previous operation, so we try to get
            some from the network. */
        err = netconn_recv_udp_raw_netbuf_flags(sock.conn, &buf, apiflags);
        LWIP_DEBUGF(SOCKETS_DEBUG,
                    ("lwip_recvfrom_udp_raw[UDP/RAW]: netconn_recv err=%d, netbuf=%p\n", err, (void *)buf));

        if (err != ERR_OK) {
            return err;
        }
        LWIP_ASSERT("buf != NULL", buf != NULL);
        sock.lastdata.netbuf = buf;
    }
    buflen = buf->p->tot_len;
    LWIP_DEBUGF(SOCKETS_DEBUG, ("lwip_recvfrom_udp_raw: buflen=%" U16_F "\n", buflen));

    copied = 0;
    /* copy the pbuf payload into the iovs */
    for (i = 0; (i < msg->msg_iovlen) && (copied < buflen); i++) {
        u16_t len_left = (u16_t)(buflen - copied);
        if (msg->msg_iov[i].iov_len > len_left) {
            copylen = len_left;
        } else {
            copylen = (u16_t)msg->msg_iov[i].iov_len;
        }

        /* copy the contents of the received buffer into
            the supplied memory buffer */
        pbuf_copy_partial(buf->p, (u8_t *)msg->msg_iov[i].iov_base, copylen, copied);
        copied = (u16_t)(copied + copylen);
    }

    /* Check to see from where the data was.*/
#if !SOCKETS_DEBUG
    if (msg->msg_name && msg->msg_namelen)
#endif /* !SOCKETS_DEBUG */
    {
        LWIP_DEBUGF(SOCKETS_DEBUG, ("lwip_recvfrom_udp_raw(%d):  addr=", dbg_s));
        ip_addr_debug_print_val(SOCKETS_DEBUG, *netbuf_fromaddr(buf));
        LWIP_DEBUGF(SOCKETS_DEBUG, (" port=%" U16_F " len=%d\n", netbuf_fromport(buf), copied));
        if (msg->msg_name && msg->msg_namelen) {
            lwip_sock_make_addr(sock.conn, netbuf_fromaddr(buf), netbuf_fromport(buf), (struct sockaddr *)msg->msg_name,
                                &msg->msg_namelen);
        }
    }

    /* Initialize flag output */
    msg->msg_flags = 0;

    if (msg->msg_control) {
        u8_t wrote_msg = 0;
#if LWIP_NETBUF_RECVINFO
        /* Check if packet info was recorded */
        if (buf->flags & NETBUF_FLAG_DESTADDR) {
            if (IP_IS_V4(&buf->toaddr)) {
#if LWIP_IPV4
                if (msg->msg_controllen >= CMSG_SPACE(sizeof(struct in_pktinfo))) {
                    struct cmsghdr *chdr = CMSG_FIRSTHDR(msg); /* This will always return a header!! */
                    struct in_pktinfo *pkti = (struct in_pktinfo *)CMSG_DATA(chdr);
                    chdr->cmsg_level = IPPROTO_IP;
                    chdr->cmsg_type = IP_PKTINFO;
                    chdr->cmsg_len = CMSG_LEN(sizeof(struct in_pktinfo));
                    pkti->ipi_ifindex = buf->p->if_idx;
                    inet_addr_from_ip4addr(&pkti->ipi_addr, ip_2_ip4(netbuf_destaddr(buf)));
                    msg->msg_controllen = CMSG_SPACE(sizeof(struct in_pktinfo));
                    wrote_msg = 1;
                } else {
                    msg->msg_flags |= MSG_CTRUNC;
                }
#endif /* LWIP_IPV4 */
            }
        }
#endif /* LWIP_NETBUF_RECVINFO */

        if (!wrote_msg) {
            msg->msg_controllen = 0;
        }
    }

    /* If we don't peek the incoming message: zero lastdata pointer and free the netbuf */
    if ((flags & MSG_PEEK) == 0) {
        sock.lastdata.netbuf = NULL;
        netbuf_delete(buf);
    }
    if (datagram_len) {
        *datagram_len = buflen;
    }
    return ERR_OK;
}

result<void> lwip_socket_device::install() noexcept {
    os::event done_event;
    tcpip_init(lwip_tcpip_init_done, &done_event);
    return done_event.wait();
}

result<void> lwip_socket_device::socket(file &file, int domain, int type, int protocol) noexcept {
    netconn_type netc_type;
    u8_t netc_proto = 0;

    // 1. Check domain
    switch (domain) {
    case AF_INET:
        break;
    default:
        return err(error_code::invalid_argument);
    }

    // 2. Check type
    switch (type) {
    case SOCK_STREAM:
        netc_type = NETCONN_TCP;
        break;
    case SOCK_DGRAM:
        netc_type = NETCONN_UDP;
        break;
    case SOCK_RAW: {
        netc_type = NETCONN_RAW;
        switch (protocol) {
        case IPPROTO_ICMP:
            netc_proto = IP_PROTO_ICMP;
            break;
        default:
            return err(error_code::invalid_argument);
        }
        break;
    }
    default:
        return err(error_code::invalid_argument);
    }

    auto netc = netconn_new_with_proto_and_callback(netc_type, netc_proto, nullptr);
    if (!netc)
        return err(error_code::out_of_memory);
    file.construct_data<lwip_sock>(netc);
    return ok();
}

result<void> lwip_socket_device::setsockopt(file &file, int level, int optname, const void *optval,
                                            socklen_t optlen) noexcept {
    auto &sock = file.data<lwip_sock>();
    tcpip_lock lock;
    switch (level) {
    /* Level: SOL_SOCKET */
    case SOL_SOCKET:
        switch (optname) {
        case SO_RCVTIMEO: {
            long ms_long;
            SOCKOPT_CHECK_OPTLEN(sock.conn, optlen, SO_SNDRCVTIMEO_OPTTYPE);
            ms_long = SO_SNDRCVTIMEO_GET_MS(optval);
            if (ms_long < 0) {
                return err(error_code::invalid_argument);
            }
            netconn_set_recvtimeout(sock.conn, (u32_t)ms_long);
            return ok();
        }
        default:
            break;
        }
    }
    return err(error_code::not_supported);
}

result<size_t> lwip_socket_device::sendto(file &file, const void *data, size_t size, int flags,
                                          const struct sockaddr *to, socklen_t tolen) noexcept {
    auto &sock = file.data<lwip_sock>();
    if (size > std::min<size_t>(0xFFFF, SSIZE_MAX)) {
        /* cannot fit into one datagram (at least for us) */
        return err(error_code::message_too_long);
    }

    auto short_size = (uint16_t)size;
    if (!(((to == NULL) && (tolen == 0)) ||
          (IS_SOCK_ADDR_LEN_VALID(tolen) &&
           ((to != NULL) && (IS_SOCK_ADDR_TYPE_VALID(to) && IS_SOCK_ADDR_ALIGNED(to)))))) {
        return err(error_code::invalid_argument);
    }

    netbuf buf;
    uint16_t remote_port;
    /* initialize a buffer */
    buf.p = buf.ptr = NULL;
#if LWIP_CHECKSUM_ON_COPY
    buf.flags = 0;
#endif /* LWIP_CHECKSUM_ON_COPY */
    if (to) {
        SOCKADDR_TO_IPADDR_PORT(to, &buf.addr, remote_port);
    } else {
        remote_port = 0;
        ip_addr_set_any(NETCONNTYPE_ISIPV6(netconn_type(sock.conn)), &buf.addr);
    }
    netbuf_fromport(&buf) = remote_port;

    if (netbuf_ref(&buf, data, short_size) != ERR_OK)
        return err(error_code::out_of_memory);
    defer_guard netbuf_defer([&] { netbuf_free(&buf); });

    /* Dual-stack: Unmap IPv4 mapped IPv6 addresses */
    if (IP_IS_V6_VAL(buf.addr) && ip6_addr_isipv4mappedipv6(ip_2_ip6(&buf.addr))) {
        unmap_ipv4_mapped_ipv6(ip_2_ip4(&buf.addr), ip_2_ip6(&buf.addr));
        IP_SET_TYPE_VAL(buf.addr, IPADDR_TYPE_V4);
    }
    /* send the data */
    return netconn_send(sock.conn, &buf) == ERR_OK ? ok(short_size) : err(error_code::io_error);
}

result<size_t> lwip_socket_device::recvfrom(file &file, void *mem, size_t len, int flags, struct sockaddr *from,
                                            socklen_t *fromlen) noexcept {
    auto &sock = file.data<lwip_sock>();
    u16_t datagram_len = 0;
    struct iovec vec;
    struct msghdr msg;
    vec.iov_base = mem;
    vec.iov_len = len;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    msg.msg_flags = 0;
    msg.msg_iov = &vec;
    msg.msg_iovlen = 1;
    msg.msg_name = from;
    msg.msg_namelen = (fromlen ? *fromlen : 0);
    if (lwip_recvfrom_udp_raw(sock, flags, &msg, &datagram_len, 0) != ERR_OK) {
        return err(error_code::io_error);
    }

    auto ret = (ssize_t)LWIP_MIN(LWIP_MIN(len, datagram_len), SSIZE_MAX);
    if (fromlen) {
        *fromlen = msg.msg_namelen;
    }
    return ok(ret);
}

result<void> lwip_socket_driver::install_device(lwip_socket_device &device) noexcept {
    try_(device.install());
    try_(io::attach_device(device));
    return ok();
}
