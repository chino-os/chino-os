// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "io_manager.h"
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

template <io_frame_generic_kind Minor, auto FastCall, class... TArgs>
auto dispatch_io_fast_slow(file &file, TArgs &&...args) noexcept
    -> decltype((std::declval<device>().*FastCall)(file, std::forward<TArgs>(args)...)) {
    file.event().reset();
    auto r = (file.device().*FastCall)(file, std::forward<TArgs>(args)...);
    if (r.is_ok()) {
        return r;
    }
    auto errcode = r.unwrap_err();
    if (errcode == error_code::slow_io) {
        io_request irp(make_io_frame_kind(io_frame_major_kind::generic, Minor), file);
        try_var(frame, irp.current_frame());
        frame->params<io_frame_major_kind::generic, Minor>() = {std::forward<TArgs>(args)...};
        try_(irp.queue());
        r = irp.wait<typename std::decay_t<decltype(r)>::value_type>();
        if (r.is_ok()) {
            return r;
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
    return ok();
}

result<object_ptr<file>> io::open_file(access_mask desired_access, std::string_view path,
                                       create_disposition disposition) noexcept {
    try_var(d, find_device(path));
    try_var(f, file_table_.allocate());
    try_(open_file(*f.first, desired_access, *d.first, d.second, disposition));
    return ok(f.first);
}

result<void> io::open_file(file &file, access_mask desired_access, device &device, std::string_view path,
                           create_disposition disposition) noexcept {
    file.prepare_to_open(device);
    return dispatch_io_fast_slow<io_frame_generic_kind::open, &device::fast_open>(file, path, disposition)
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
    return dispatch_io_fast_slow<io_frame_generic_kind::read, &device::fast_read>(file, buffer, offset);
}

result<size_t> io::write_file(file &file, std::span<const std::byte> buffer, std::optional<size_t> offset) noexcept {
    return dispatch_io_fast_slow<io_frame_generic_kind::write, &device::fast_write>(file, buffer, offset);
}

result<int> io::control_file(file &file, control_code_t code, void *arg) noexcept {
    return dispatch_io_fast_slow<io_frame_generic_kind::control, &device::fast_control>(file, code, arg);
}

result<void> io::allocate_console() noexcept { return ok(); }
