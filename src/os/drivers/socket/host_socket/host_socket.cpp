// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "../../../hal/archs/emulator/emulator.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define INCL_WINSOCK_API_TYPEDEFS 1
#include <WinSock2.h>

#include "host_socket.h"
#include <chino/os/kernel/io.h>
#include <chino/os/kernel/ps.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::kernel::io;
using namespace chino::os::drivers;

static LPFN_SETSOCKOPT pfn_setsockopt;

result<void> host_socket_device::install() noexcept {
    WSADATA wsaData;
    auto wVersionRequested = MAKEWORD(2, 2);
    TRY_WSA_IF_NOT(WSAStartup(wVersionRequested, &wsaData) == 0);
    auto wsaModule = LoadLibrary(L"WS2_32.dll");
    pfn_setsockopt = (LPFN_SETSOCKOPT)GetProcAddress(wsaModule, "setsockopt");
    return ok();
}

result<void> host_socket_device::close(file &file) noexcept {
    TRY_WSA_IF_NOT(closesocket(file.data<SOCKET>()) == 0);
    return ok();
}

result<void> host_socket_device::socket(file &file, int domain, int type, int protocol) noexcept {
    auto sock = WSASocket(domain, type, protocol, nullptr, 0, 0);
    TRY_WSA_IF_NOT(sock != INVALID_SOCKET);
    file.construct_data<SOCKET>(sock);
    return ok();
}

result<void> host_socket_device::setsockopt(file &file, int level, int optname, const void *optval,
                                            socklen_t optlen) noexcept {
    auto sock = file.data<SOCKET>();
    switch (optname) {
    case SO_RCVTIMEO: {
        auto src_time = (timeval *)optval;
        DWORD dest_time = src_time->tv_sec * 1000 + src_time->tv_usec / 1000000;
        TRY_WSA_IF_NOT(pfn_setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&dest_time, sizeof(dest_time)) == 0);
        break;
    }
    default:
        return err(error_code::not_supported);
    }
    return ok();
}

result<size_t> host_socket_device::sendto(file &file, const void *data, size_t size, int flags,
                                          const struct sockaddr *to, socklen_t tolen) noexcept {
    WSABUF buf{.len = (ULONG)size, .buf = (CHAR *)data};
    DWORD bytes_sent;
    TRY_WSA_IF_NOT(WSASendTo(file.data<SOCKET>(), &buf, 1, &bytes_sent, flags, to, tolen, nullptr, nullptr) == 0);
    return ok(bytes_sent);
}

result<size_t> host_socket_device::recvfrom(file &file, void *mem, size_t len, int flags, struct sockaddr *from,
                                            socklen_t *fromlen) noexcept {
    WSABUF buf{.len = (ULONG)len, .buf = (CHAR *)mem};
    DWORD bytes_recv;
    INT wsa_fromlen = *fromlen;
    DWORD wsa_flags = flags;
    TRY_WSA_IF_NOT(
        WSARecvFrom(file.data<SOCKET>(), &buf, 1, &bytes_recv, &wsa_flags, from, &wsa_fromlen, nullptr, nullptr) == 0);
    *fromlen = wsa_fromlen;
    return ok(bytes_recv);
}

result<void> host_socket_driver::install_device(host_socket_device &device) noexcept {
    try_(device.install());
    try_(io::attach_device(device));
    return ok();
}
