// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "io_manager.h"
#include "../ke/ke_services.h"
#include "../ob/directory.h"
#include "../ps/task/process.h"
#include <chino/conf/board_init.inl>
#include <chino/os/kernel/kd.h>
#include <chino/os/kernel/ob.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::kernel::io;
using namespace std::string_view_literals;

#define BUFFER_TO_IOVS(buffer, iovs)                                                                                   \
    iovec iovs##0 {.iov_base = (void *)buffer.data(), .iov_len = buffer.size_bytes()};                                 \
    std::span<const iovec> iovs(&iovs##0, 1)

namespace {
class dev_directory : public ob::directory {
  public:
    std::string_view name() const noexcept override { return "dev"sv; }
};

constinit dev_directory dev_directory_;
constinit ps::irq_spin_lock device_work_list_lock_;
constinit os::event device_work_list_avail_event_;
constinit intrusive_list<device, &device::work_list_node> device_work_list_;
constinit object_pool<io::file> file_table_;
constinit object_ptr<stdio_device> default_stdio_device_;

template <io_frame_generic_kind Minor, auto FastCall, class... TArgs>
auto dispatch_io_fast_slow(file &file, async_io_result *async_io_result, TArgs &&...args) noexcept
    -> decltype((std::declval<device>().*FastCall)(file, std::forward<TArgs>(args)...)) {
    file.event().reset();
    auto r = (file.device().*FastCall)(file, std::forward<TArgs>(args)...);
    if (r.is_ok()) {
        return r;
    }
    auto errcode = r.unwrap_err();
    if (errcode == error_code::slow_io) {
        try_var(io_block, async_io_result ? async_io_block::allocate(*async_io_result) : ok<async_io_block *>(nullptr));
        try_var(irp, io_request::allocate(io_block, make_io_frame_kind(io_frame_major_kind::generic, Minor), file));
        try_var(frame, irp->current_frame());
        frame->template params<io_frame_major_kind::generic, Minor>() = {std::forward<TArgs>(args)...};
        irp->queue();
        if (async_io_result) {
            return err(error_code::io_pending);
        } else {
            r = irp->template wait<typename std::decay_t<decltype(r)>::value_type>();
            if (r.is_ok()) {
                return r;
            }
        }
    }
    return err(r.unwrap_err());
}

result<std::pair<object_ptr<device>, std::string_view>> find_device(std::string_view path) {
    // 1. Search by absolute path
    auto result = ob::lookup_object_partial<device>(path);
    if (result.is_ok()) {
        return result;
    } else if (result.unwrap_err() == error_code::not_found && !path.empty() && path[0] == ob::directory_separator) {
        // 2. Search in fs0
        try_var(fs0, ob::lookup_object<device>("/dev/fs0"));
        return ok(std::make_pair(std::move(fs0), path.substr(1)));
    }
    return err(result.unwrap_err());
}
} // namespace

template <> void object_pool<io::file>::object_pool_object::internal_release() noexcept {
    (void)file_table_.free(this);
}

result<void> io::initialize_phase1(const boot_options &options) noexcept {
    try_(ob::insert_object(dev_directory_, "/dev"));
    try_(hal::hal_install_devices());
    return ok();
}

void io::process_queued_ios(io_request *wait_irp) {
    while (true) {
        device *head;
        {
            std::unique_lock<ps::irq_spin_lock> lock(device_work_list_lock_);
            head = device_work_list_.front();
            if (head)
                device_work_list_.remove(head);
            else
                device_work_list_avail_event_.reset();
        }

        if (head) {
            if (head->process_queued_ios(wait_irp))
                return;
        } else {
            device_work_list_avail_event_.wait().expect("Wait event failed.");
        }
    }
}

object_ptr<stdio_device> io::default_stdio_device() noexcept { return default_stdio_device_; }

int io::io_worker_main(void *) noexcept {
    process_queued_ios(nullptr);
    CHINO_UNREACHABLE();
}

void io::register_device_process_io(device &device) noexcept {
    std::unique_lock<ps::irq_spin_lock> lock(device_work_list_lock_);
    if (!device.work_list_node.in_list()) {
        device_work_list_.push_back(&device);
        device_work_list_avail_event_.notify_one();
    }
}

result<void> io::attach_device(device &device) noexcept {
    try_(dev_directory_.insert(device, device.name()));
    // Handle special device
    if (device.is_a(object_kind_stdio_device)) {
        kassert(default_stdio_device_.empty());
        default_stdio_device_ = static_cast<stdio_device *>(&device);
    } else if (device.is_a(object_kind_socket_device)) {
        try_(io::initialize_socket_manager(device));
    } else if (device.is_a(object_kind_ble_device)) {
        try_(io::initialize_ble_manager(device));
    }
    return ok();
}

result<object_ptr<file>> io::open_file(access_mask desired_access, device &device, std::string_view path,
                                       create_disposition disposition) noexcept {
    try_var(f, file_table_.allocate());
    try_(open_file(*f.first, desired_access, device, path, disposition));
    return ok(f.first);
}

result<object_ptr<file>> io::open_file(access_mask desired_access, std::string_view path,
                                       create_disposition disposition) noexcept {
    try_var(d, find_device(path));
    return open_file(desired_access, *d.first, d.second, disposition);
}

result<void> io::open_file(file &file, access_mask desired_access, device &device, std::string_view path,
                           create_disposition disposition) noexcept {
    file.prepare_to_open(device);
    return dispatch_io_fast_slow<io_frame_generic_kind::open, &device::fast_open>(file, nullptr, path, disposition)
        .map_err([&](error_code e) {
            file.failed_to_open();
            return e;
        });
}

result<void> io::open_file(file &file, access_mask desired_access, std::string_view path,
                           create_disposition disposition) noexcept {
    try_var(r, find_device(path));
    return open_file(file, desired_access, *r.first, r.second, disposition);
}

result<void> io::close_file(file &file) noexcept { return file.device().close(file); }

result<size_t> io::read_file(file &file, std::span<std::byte> buffer, std::optional<size_t> offset) noexcept {
    return dispatch_io_fast_slow<io_frame_generic_kind::read, &device::fast_read>(file, nullptr, buffer, offset);
}

result<void> io::read_file_async(file &file, std::span<std::byte> buffer, size_t offset,
                                 async_io_result &result) noexcept {
    try_var(bytes_read,
            dispatch_io_fast_slow<io_frame_generic_kind::read, &device::fast_read>(file, &result, buffer, offset));
    result.bytes_transferred = bytes_read;
    return ok();
}

result<size_t> io::write_file(file &file, std::span<const std::byte> buffer, std::optional<size_t> offset) noexcept {
    return dispatch_io_fast_slow<io_frame_generic_kind::write, &device::fast_write>(file, nullptr, buffer, offset);
}

result<int> io::control_file(file &file, control_code_t code, void *arg) noexcept {
    return dispatch_io_fast_slow<io_frame_generic_kind::control, &device::fast_control>(file, nullptr, code, arg);
}

result<void> io::allocate_console() noexcept { return ok(); }

int kernel_ke_service_mt::open(const char *pathname, int flags, mode_t mode) noexcept {
    return wrap_posix<ssize_t>([=]() -> result<int> {
        try_var(file, io::open_file(access_mask::generic_all, pathname, create_disposition::open_existing));
        try_var(handle, ob::alloc_handle(*file, access_mask::generic_all));
        return ok(handle.second);
    });
}

int kernel_ke_service_mt::close(int fd) noexcept {
    return wrap_posix<void>([=]() -> result<void> { return ob::close_handle(fd); });
}

ssize_t kernel_ke_service_mt::read(int __fd, void *__buf, size_t __nbyte) noexcept {
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

ssize_t kernel_ke_service_mt::write(int __fd, const void *__buf, size_t __nbyte) noexcept {
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

int kernel_ke_service_mt::ioctl(int __fd, int req, void *arg) noexcept {
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

result<async_io_result *> kernel_ke_service_mt::wait_queued_io() noexcept { return io::io_wait_queued_io(); }

result<void> kernel_ke_service_mt::read_async(int fd, std::span<std::byte> buffer, size_t offset,
                                              async_io_result &result) noexcept {
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
