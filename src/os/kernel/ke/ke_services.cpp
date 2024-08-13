// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "ke_services.h"
#include "../io/io_manager.h"
#include <chino/os/hal/chip.h>
#include <chino/os/kernel/io.h>
#include <chino/os/kernel/ob.h>
#include <chino/os/kernel/ps.h>

#ifdef CHINO_EMULATOR
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;

int kernel::to_errno(error_code code) {
    switch (code) {
    case chino::error_code::success:
        return 0;
    case chino::error_code::argument_null:
        return EINVAL;
    case chino::error_code::invalid_argument:
        return EINVAL;
    case chino::error_code::out_of_memory:
        return ENOMEM;
    case chino::error_code::not_found:
        return ENOENT;
    case chino::error_code::unavailable:
        return EBUSY;
    case chino::error_code::key_already_exists:
        return EEXIST;
    case chino::error_code::not_implemented:
        return ENOTSUP;
    case chino::error_code::not_supported:
        return ENOTSUP;
    case chino::error_code::invalid_path:
        return ENOENT;
    case chino::error_code::insufficient_buffer:
        return ENOMEM;
    case chino::error_code::io_error:
        return EIO;
    case chino::error_code::bad_cast:
        return EBADF;
    case chino::error_code::timeout:
        return ETIMEDOUT;
    case chino::error_code::message_too_long:
        return EMSGSIZE;
    case chino::error_code::no_such_device:
        return ENODEV;
    default:
        return EFAULT;
    }
}

int kernel_ke_service_mt::errno_() noexcept { return errno; }

struct user_ke_services_mt : kernel_ke_service_mt {
  public:
    user_ke_services_mt() noexcept {
        io::open_file(stdio_, access_mask::generic_all, *io::default_stdio_device(), {},
                      create_disposition::open_existing)
            .expect("Open console failed.");
    }

    ssize_t read(int __fd, void *__buf, size_t __nbyte) noexcept override {
        if (__fd == STDIN_FILENO || __fd == STDERR_FILENO) {
            return wrap_posix<ssize_t>([=, this]() -> result<ssize_t> {
                return io::read_file(stdio_, {reinterpret_cast<std::byte *>(__buf), __nbyte});
            });
        }
        return kernel_ke_service_mt::read(__fd, __buf, __nbyte);
    }

    ssize_t write(int __fd, const void *__buf, size_t __nbyte) noexcept override {
        if (__fd == STDOUT_FILENO) {
            return wrap_posix<ssize_t>([=, this]() -> result<ssize_t> {
                return io::write_file(stdio_, {reinterpret_cast<const std::byte *>(__buf), __nbyte});
            });
        }
        return kernel_ke_service_mt::write(__fd, __buf, __nbyte);
    }

    result<void> read_async(int fd, std::span<std::byte> buffer, size_t offset,
                            async_io_result &result) noexcept override {
        if (fd == STDIN_FILENO)
            return io::read_file_async(stdio_, buffer, offset, result);
        return kernel_ke_service_mt::read_async(fd, buffer, offset, result);
    }

  private:
    kernel::io::file stdio_;
};

constinit static kernel_ke_service_mt kernel_ke_mt;

i_ke_services &os::ke_services() noexcept { return kernel_ke_mt; }

result<void> kernel::initialize_ke_services() noexcept {
#ifdef CHINO_ARCH_EMULATOR
    VirtualAlloc((LPVOID)ke_services_address, sizeof(user_ke_services_mt), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    std::construct_at(reinterpret_cast<user_ke_services_mt *>(ke_services_address));
    VirtualProtect((LPVOID)ke_services_address, sizeof(user_ke_services_mt), PAGE_READONLY, nullptr);
#endif
    return ok();
}
