// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "../../ke/ke_services.h"
#include "../../ps/task/process.h"
#include "../io_manager.h"
#include <chino/os/kernel/io/devices/socket_device.h>
#include <chino/os/kernel/kd.h>
#include <chino/os/kernel/ob.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::kernel::io;
using namespace std::string_view_literals;

constinit static object_ptr<socket_device> default_socket_device_;

#define GET_SOCK_DEV(fd)                                                                                               \
    try_var(f, ob::reference_object<file>(sockfd));                                                                    \
    socket_device *dev;                                                                                                \
    if (f->device().is_a(socket_device::kind()))                                                                       \
        dev = static_cast<socket_device *>(&f->device());                                                              \
    else                                                                                                               \
        return err(error_code::bad_cast)

result<void> io::initialize_socket_manager(device &socket_device) noexcept {
    kassert(default_socket_device_.empty());
    default_socket_device_ = static_cast<io::socket_device *>(&socket_device);
    return ok();
}

int kernel_ke_service_mt::socket(int domain, int type, int protocol) noexcept {
    return wrap_posix<int>([&]() -> result<int> {
        if (default_socket_device_.empty())
            return err(error_code::no_such_device);
        try_var(file,
                io::open_file(access_mask::generic_all, *default_socket_device_, {}, create_disposition::create_new));
        try_(default_socket_device_->socket(*file, domain, type, protocol));
        try_var(handle, ob::alloc_handle(*file, access_mask::generic_all));
        return ok(handle.second);
    });
}

int kernel_ke_service_mt::setsockopt(int sockfd, int level, int optname, const void *optval,
                                     socklen_t optlen) noexcept {
    return wrap_posix<void>([&]() -> result<void> {
        GET_SOCK_DEV(sockfd);
        return dev->setsockopt(*f, level, optname, optval, optlen);
    });
}

ssize_t kernel_ke_service_mt::sendto(int sockfd, const void *data, size_t size, int flags, const struct sockaddr *to,
                                     socklen_t tolen) noexcept {
    return wrap_posix<ssize_t>([&]() -> result<size_t> {
        GET_SOCK_DEV(sockfd);
        return dev->sendto(*f, data, size, flags, to, tolen);
    });
}

ssize_t kernel_ke_service_mt::recvfrom(int sockfd, void *mem, size_t len, int flags, struct sockaddr *from,
                                       socklen_t *fromlen) noexcept {
    return wrap_posix<ssize_t>([&]() -> result<ssize_t> {
        GET_SOCK_DEV(sockfd);
        return dev->recvfrom(*f, mem, len, flags, from, fromlen);
    });
}
