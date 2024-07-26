// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "io_manager.h"
#include "../ob/directory.h"
#include <chino/conf/board_init.inl>
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

int io::io_worker_main(void *) noexcept {
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

        if (head)
            head->process_queued_ios();
        else
            device_work_list_avail_event_.wait().expect("Wait event failed.");
    }
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

result<file> io::open_file(device &device, std::string_view path, create_disposition disposition) noexcept {
    file file(device, access_mask::generic_all);
    try_(device.fast_open(file, path, disposition));
    return ok(std::move(file));
}

result<file> io::open_file(std::string_view path, create_disposition disposition) noexcept {
    // 1. Search by absolute path
    auto result = ob::lookup_object_partial<device>(path);

    // 2. Search in fs0
    if (result.is_err()) {
        if (result.unwrap_err() == error_code::not_found && !path.empty() && path[0] == ob::directory_separator) {
            try_var(fs0, ob::lookup_object<device>("/dev/fs0"));
            return open_file(*fs0, path.substr(1), disposition);
        } else {
            return err(result.unwrap_err());
        }
    }

    return open_file(*result.unwrap().first, result.unwrap().second, disposition);
}

result<void> io::close_file(file &file) noexcept {
    TRY_GET_DEVICE(file);
    return dev.close(file);
}

result<size_t> io::read_file(file &file, std::span<std::byte> buffer, std::optional<size_t> offset) noexcept {
    TRY_GET_DEVICE(file);
    return dev.fast_read(file, buffer, offset);
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
