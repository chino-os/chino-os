// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "ioapi.h"
#include <sys/socket.h>
#include <sys/unistd.h>

namespace chino::os {
struct i_ke_services {
    virtual int errno_() noexcept = 0;
    virtual int open(const char *pathname, int flags, mode_t mode) noexcept = 0;
    virtual int close(int fd) noexcept = 0;
    virtual ssize_t read(int __fd, void *__buf, size_t __nbyte) noexcept = 0;
    virtual ssize_t write(int __fd, const void *__buf, size_t __nbyte) noexcept = 0;
    virtual int ioctl(int fd, int req, void *arg) noexcept = 0;

    virtual result<void> atomic_wait(std::atomic<uint32_t> &atomic, uint32_t old,
                                     std::optional<std::chrono::milliseconds> timeout = std::nullopt) noexcept = 0;
    virtual void atomic_notify_one(std::atomic<uint32_t> &atomic) noexcept = 0;
    virtual void atomic_notify_all(std::atomic<uint32_t> &atomic) noexcept = 0;

    virtual result<async_io_result *> wait_queued_io() noexcept = 0;
    virtual result<void> read_async(int fd, std::span<std::byte> buffer, size_t offset,
                                    async_io_result &result) noexcept = 0;

    virtual int socket(int domain, int type, int protocol) noexcept = 0;
    virtual int setsockopt(int sockfd, int level, int option, const void *value, socklen_t value_len) noexcept = 0;
    virtual ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const sockaddr *to,
                           socklen_t tolen) noexcept = 0;
    virtual ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, sockaddr *from,
                             socklen_t *fromlen) noexcept = 0;

    virtual int nanosleep(const timespec *rqtp, timespec *rmtp) noexcept = 0;
    virtual int clock_gettime(clockid_t clock_id, timespec *tp) noexcept = 0;
};

inline static uintptr_t ke_services_address = 0x800000000;

i_ke_services &ke_services() noexcept;
} // namespace chino::os
