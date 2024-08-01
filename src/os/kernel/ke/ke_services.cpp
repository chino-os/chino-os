// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "ke_services.h"
#include "../ps/sched/scheduler.h"
#include <chino/os/hal/chip.h>
#include <chino/os/kernel/io.h>
#include <chino/os/kernel/ob.h>
#include <chino/os/kernel/ps.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;

namespace {
int to_errno(error_code code) {
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
    default:
        return EFAULT;
    }
}

template <class T, class Callable> auto wrap_posix(Callable &&callable) noexcept {
    auto r = callable();
    if (r.is_ok()) {
        if constexpr (std::is_void_v<T>) {
            return 0;
        } else {
            return static_cast<T>(r.unwrap());
        }
    } else {
        errno = to_errno(r.unwrap_err());
        return static_cast<std::conditional_t<std::is_void_v<T>, int, T>>(-1);
    }
}
} // namespace

struct kernel_ke_service_mt : i_ke_services {
  public:
    int errno_() noexcept override { return errno; }

    int open(const char *pathname, int flags, mode_t mode) noexcept override {
        return wrap_posix<ssize_t>([=]() -> result<int> {
            try_var(file, io::open_file(access_mask::generic_all, pathname, create_disposition::open_existing));
            try_var(handle, ob::alloc_handle(*file, access_mask::generic_all));
            return ok(handle.second);
        });
    }

    int close(int fd) noexcept override {
        return wrap_posix<void>([=]() -> result<void> { return ob::close_handle(fd); });
    }

    ssize_t read(int __fd, void *__buf, size_t __nbyte) noexcept override {
        return wrap_posix<ssize_t>([=]() -> result<ssize_t> {
            switch (__fd) {
            case STDIN_FILENO:
                return err(error_code::not_supported);
            case STDOUT_FILENO:
            case STDERR_FILENO:
                return err(error_code::bad_cast);
            default:
                try_var(file, ob::reference_object<io::file>(__fd));
                return io::read_file(*file, {reinterpret_cast<std::byte *>(__buf), __nbyte});
            }
        });
    }

    ssize_t write(int __fd, const void *__buf, size_t __nbyte) noexcept override {
        return wrap_posix<ssize_t>([=]() -> result<ssize_t> {
            switch (__fd) {
            case STDIN_FILENO:
                return err(error_code::bad_cast);
            case STDOUT_FILENO:
            case STDERR_FILENO:
                hal::chip_t::debug_print(reinterpret_cast<const char *>(__buf));
                return ok(__nbyte);
            default:
                try_var(file, ob::reference_object<io::file>(__fd));
                return io::write_file(*file, {reinterpret_cast<const std::byte *>(__buf), __nbyte});
            }
        });
    }

    int ioctl(int __fd, int req, void *arg) noexcept override {
        return wrap_posix<int>([=]() -> result<int> {
            switch (__fd) {
            case STDIN_FILENO:
            case STDOUT_FILENO:
            case STDERR_FILENO:
                return err(error_code::bad_cast);
            default:
                try_var(file, ob::reference_object<io::file>(__fd));
                return io::control_file(*file, req, arg);
            }
        });
    }

    result<void> atomic_wait(std::atomic<uint32_t> &atomic, uint32_t old,
                             std::optional<std::chrono::milliseconds> timeout = std::nullopt) noexcept override {
        if (atomic.load(std::memory_order_acquire) != old) {
            // 1. Fast path
            return ok();
        } else {
            // 2. Slow path
            return ps::scheduler::current().block_current_thread(atomic, old, timeout);
        }
    }

    void atomic_notify_one(std::atomic<uint32_t> &atomic) noexcept override {
        ps::scheduler::unblock_threads(&atomic, false);
    }

    void atomic_notify_all(std::atomic<uint32_t> &atomic) noexcept override {
        ps::scheduler::unblock_threads(&atomic, true);
    }

    result<async_io_result *> wait_queued_io() noexcept override { return io::io_wait_queued_io(); }

    result<void> read_async(int fd, std::span<std::byte> buffer, size_t offset,
                            async_io_result &result) noexcept override {
        switch (fd) {
        case STDIN_FILENO:
            return err(error_code::not_supported);
        case STDOUT_FILENO:
        case STDERR_FILENO:
            return err(error_code::bad_cast);
        default:
            try_var(file, ob::reference_object<io::file>(fd));
            return io::read_file_async(*file, buffer, offset, result);
        }
    }
};

struct user_ke_services_mt : kernel_ke_service_mt {
  public:
    user_ke_services_mt() noexcept {
        io::open_file(stdio_, access_mask::generic_all, "/dev/console", create_disposition::open_existing)
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
