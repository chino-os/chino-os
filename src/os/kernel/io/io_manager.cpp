// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "io_manager.h"
#include "../ob/directory.h"
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

#define TRY_GET_DEVICE(file)                                                                                           \
    if (!file.object().is_a(os::kernel::io::device::kind())) [[unlikely]]                                              \
        return err(error_code::not_supported);                                                                         \
    auto &dev = static_cast<os::kernel::io::device &>(file.object())

namespace {
class dev_directory : public ob::directory {
  public:
    std::string_view name() const noexcept override { return "dev"sv; }
};

constinit dev_directory dev_directory_;
constinit ps::irq_spin_lock device_work_list_lock_;
constinit ps::event device_work_list_avail_event_;
constinit intrusive_list<device, &device::work_list_node> device_work_list_;
} // namespace

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
    device_work_list_.push_back(&device);
    device_work_list_avail_event_.notify_all();
}

result<void> io::attach_device(device &device) noexcept {
    try_(dev_directory_.insert(device, device.name()));
    return ok();
}

result<void> io::open_file(file &file, access_mask desired_access, device &device, std::string_view path,
                           create_disposition disposition) noexcept {
    file.prepare_to_open(device, desired_access);
    auto r = device.fast_open(file, path, disposition);
    if (r.is_ok()) {
        return r;
    }
    auto errcode = r.unwrap_err();
    if (errcode == error_code::slow_io) {
        io_request irp(make_io_frame_kind(io_frame_major_kind::generic, io_frame_generic_kind::open), file);
        try_var(frame, irp.current_frame());
        frame->params<io_frame_params_generic>().open = {.path = path, .create_disposition = disposition};
        try_(irp.queue());
        r = irp.wait();
        if (r.is_ok()) {
            return r;
        }
    }
    file.failed_to_open();
    return err(r.unwrap_err());
}

result<void> io::open_file(file &file, access_mask desired_access, std::string_view path,
                           create_disposition disposition) noexcept {
    // 1. Search by absolute path
    auto result = ob::lookup_object_partial<device>(path);

    // 2. Search in fs0
    if (result.is_err()) {
        if (result.unwrap_err() == error_code::not_found && !path.empty() && path[0] == ob::directory_separator) {
            try_var(fs0, ob::lookup_object<device>("/dev/fs0"));
            return open_file(file, desired_access, *fs0, path.substr(1), disposition);
        } else {
            return err(result.unwrap_err());
        }
    }

    return open_file(file, desired_access, *result.unwrap().first, result.unwrap().second, disposition);
}

result<void> io::close_file(file &file) noexcept {
    TRY_GET_DEVICE(file);
    return dev.close(file);
}

result<size_t> io::read_file(file &file, std::span<std::byte> buffer, std::optional<size_t> offset) noexcept {
    TRY_GET_DEVICE(file);
    file.event().reset();
    auto r = dev.fast_read(file, buffer, offset);
    if (r.is_ok()) {
        return r;
    }
    auto errcode = r.unwrap_err();
    if (errcode == error_code::slow_io) {
        io_request irp(make_io_frame_kind(io_frame_major_kind::generic, io_frame_generic_kind::read), file);
        try_var(frame, irp.current_frame());
        frame->params<io_frame_params_generic>().read = {.buffer = buffer, .offset = offset};
        try_(irp.queue());
        r = irp.wait<size_t>();
        if (r.is_ok()) {
            return r;
        }
    }
    return err(r.unwrap_err());
}

result<size_t> io::write_file(file &file, std::span<const std::byte> buffer, std::optional<size_t> offset) noexcept {
    TRY_GET_DEVICE(file);
    return dev.fast_write(file, buffer, offset);
}

result<int> io::control_file(file &file, int request, void *arg) noexcept {
    TRY_GET_DEVICE(file);
    return dev.fast_control(file, request, arg);
}

result<void> io::allocate_console() noexcept { return ok(); }
