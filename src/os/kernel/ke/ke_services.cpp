// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "ke_services.h"
#include <chino/os/kernel/io.h>
#include <chino/os/kernel/ob.h>
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

struct ke_services_mt : i_ke_services {
  public:
    ke_services_mt() noexcept : stdio_(io::open_file("/dev/console", create_disposition::open_existing).unwrap()) {}

    int errno_() noexcept override { return errno; }

    int open(const char *pathname, int flags, mode_t mode) noexcept override {
        return wrap_posix<ssize_t>([=]() -> result<int> {
            try_var(file, io::open_file(pathname, create_disposition::open_existing));
            return ob::insert_handle(std::move(file));
        });
    }

    int close(int fd) noexcept override {
        return wrap_posix<void>([=]() -> result<void> { return ob::close_handle(fd); });
    }

    ssize_t read(int __fd, void *__buf, size_t __nbyte) noexcept override {
        return wrap_posix<ssize_t>([=, this]() -> result<ssize_t> {
            switch (__fd) {
            case STDIN_FILENO:
                return io::read_file(stdio_, {reinterpret_cast<std::byte *>(__buf), __nbyte});
            case STDOUT_FILENO:
            case STDERR_FILENO:
                return err(error_code::bad_cast);
            default:
                try_var(file, ob::reference_handle(__fd));
                return io::read_file(*file, {reinterpret_cast<std::byte *>(__buf), __nbyte});
            }
        });
    }

    ssize_t write(int __fd, const void *__buf, size_t __nbyte) noexcept override {
        return wrap_posix<ssize_t>([=, this]() -> result<ssize_t> {
            switch (__fd) {
            case STDIN_FILENO:
                return err(error_code::bad_cast);
            case STDOUT_FILENO:
            case STDERR_FILENO:
                return io::write_file(stdio_, {reinterpret_cast<const std::byte *>(__buf), __nbyte});
            default:
                try_var(file, ob::reference_handle(__fd));
                return io::write_file(*file, {reinterpret_cast<const std::byte *>(__buf), __nbyte});
            }
        });
    }

    int vioctl(int __fd, int req, va_list ap) noexcept override {
        return wrap_posix<ssize_t>([=]() -> result<ssize_t> {
            switch (__fd) {
            case STDIN_FILENO:
            case STDOUT_FILENO:
            case STDERR_FILENO:
                return err(error_code::bad_cast);
            default:
                try_var(file, ob::reference_handle(__fd));
                return io::control_file(*file, req, ap);
            }
        });
    }

  private:
    file stdio_;
};

result<void> kernel::initialize_ke_services() noexcept {
#ifdef CHINO_ARCH_EMULATOR
    VirtualAlloc((LPVOID)ke_services_address, sizeof(ke_services_mt), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    std::construct_at(reinterpret_cast<ke_services_mt *>(ke_services_address));
    VirtualProtect((LPVOID)ke_services_address, sizeof(ke_services_mt), PAGE_READONLY, nullptr);
#endif
    return ok();
}
