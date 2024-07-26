// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include <chino/os/kernel/io.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::kernel::io;
using namespace chino::os::kernel::ps;

result<void> device::close(file &file) noexcept { return ok(); }

result<void> device::fast_open(file &file, std::string_view path, create_disposition create_disposition) noexcept {
    if (path.empty()) {
        return ok();
    }
    return err(error_code::invalid_path);
}

result<size_t> device::fast_read(file &file, std::span<std::byte> buffer, std::optional<size_t> offset) noexcept {
    return err(error_code::not_supported);
}

result<size_t> device::fast_write(file &file, std::span<const std::byte> buffer,
                                  std::optional<size_t> offset) noexcept {
    return err(error_code::not_supported);
}

result<size_t> device::fast_control(file &file, control_code_t code, void *arg) noexcept {
    return err(error_code::not_supported);
}

result<void> device::queue_io(io_request &irp) noexcept {
    {
        std::unique_lock<irq_spin_lock> lock(irps_lock_);
        irps_.push_back(&irp);
    }
    io::register_device_process_io(*this);
    return ok();
}

void device::process_queued_ios() noexcept {
    while (true) {
        io_request *head;
        {
            std::unique_lock<ps::irq_spin_lock> lock(irps_lock_);
            head = irps_.front();
            if (head)
                irps_.remove(head);
        }

        if (head) {
            current_irp_ = head;
            (void)process_io(*head);
        } else
            break;
    }
}

result<void> device::process_io(io_request &irp) noexcept {
    auto &frame = irp.current_frame();
    auto kind = frame.kind();
    if (kind.major == io_frame_major_kind::generic) {
        auto &params = frame.params<io_frame_params_generic>();
        switch ((io_frame_generic_kind)kind.minor) {
        case io_frame_generic_kind::open: {
            try_(fast_open(frame.file(), params.open.path, params.open.create_disposition));
            irp.complete(ok());
            return ok();
        }
        case io_frame_generic_kind::read: {
            try_var(bytes, fast_read(frame.file(), params.read.buffer, params.read.offset));
            irp.complete(ok(bytes));
            return ok();
        }
        case io_frame_generic_kind::write: {
            try_var(bytes, fast_write(frame.file(), params.write.buffer, params.write.offset));
            irp.complete(ok(bytes));
            return ok();
        }
        case io_frame_generic_kind::control: {
            try_var(status, fast_control(frame.file(), params.control.code, params.control.arg));
            irp.complete(ok(status));
            return ok();
        }
        }
    }
    return err(error_code::not_supported);
}

result<void> device::cancel_io(io_request &irp) noexcept { return err(error_code::not_supported); }

result<void> device::on_io_completion(io_request &irp) noexcept { return ok(); }
