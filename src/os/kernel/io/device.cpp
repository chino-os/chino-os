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
    return err(error_code::slow_io);
}

result<size_t> device::fast_write(file &file, std::span<const std::byte> buffer,
                                  std::optional<size_t> offset) noexcept {
    return err(error_code::slow_io);
}

result<size_t> device::fast_control(file &file, control_code_t code, void *arg) noexcept {
    return err(error_code::slow_io);
}

result<void> device::queue_io(io_request &irp) noexcept {
    kassert(!hal::arch_t::in_irq_handler());
    std::unique_lock<irq_spin_lock> lock(irps_lock_);
    irps_.push_back(&irp);
    if (!current_irp_)
        io::register_device_process_io(*this);
    return ok();
}

bool device::process_queued_ios(io_request *wait_irp) noexcept {
    kassert(!hal::arch_t::in_irq_handler());
    while (true) {
        io_request *irp;
        {
            std::unique_lock<ps::irq_spin_lock> lock(irps_lock_);
            irp = current_irp_;
            if (!irp) {
                irp = irps_.front();
                if (irp) {
                    current_irp_ = irp;
                    irps_.remove(irp);
                }
            }
        }

        if (irp) {
            auto last_frame = irp->current_frame().expect("Failed to get frame");
            auto r = process_io(*irp);
            if (r.is_err()) {
                irp->complete(r);
            }
            auto new_frame = irp->current_frame();
            if (new_frame.is_err() || new_frame.unwrap() != last_frame) {
                // Last frame is completed
                current_irp_ = nullptr;
            }

            if (irp->is_completed()) {
                if (irp == wait_irp) {
                    return true;
                } else {
                    continue;
                }
            } else {
                break;
            }
        } else {
            break;
        }
    }
    return false;
}

result<void> device::process_io(io_request &irp) noexcept {
    try_var(frame, irp.current_frame());
    auto kind = frame->kind();
    if (kind.major == io_frame_major_kind::generic) {
        auto &params = frame->params<io_frame_params_generic>();
        switch ((io_frame_generic_kind)kind.minor) {
        case io_frame_generic_kind::open: {
            try_(fast_open(frame->file(), params.open.path, params.open.create_disposition));
            irp.complete(ok());
            return ok();
        }
        case io_frame_generic_kind::read: {
            try_var(bytes, fast_read(frame->file(), params.read.buffer, params.read.offset));
            irp.complete(ok(bytes));
            return ok();
        }
        case io_frame_generic_kind::write: {
            try_var(bytes, fast_write(frame->file(), params.write.buffer, params.write.offset));
            irp.complete(ok(bytes));
            return ok();
        }
        case io_frame_generic_kind::control: {
            try_var(status, fast_control(frame->file(), params.control.code, params.control.arg));
            irp.complete(ok(status));
            return ok();
        }
        }
    }
    return err(error_code::not_supported);
}

result<void> device::cancel_io(io_request &irp) noexcept { return err(error_code::not_supported); }

void device::on_io_completion(io_request &irp) noexcept { irp.complete(); }
