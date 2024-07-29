// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "io.h"
#include <sys/unistd.h>

namespace chino::os::kernel {
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
};

inline static uintptr_t ke_services_address = 0x800000000;

inline i_ke_services &ke_services() noexcept { return *reinterpret_cast<i_ke_services *>(ke_services_address); }
} // namespace chino::os::kernel
