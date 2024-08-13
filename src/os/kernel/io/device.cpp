// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include <chino/os/kernel/io.h>
#include <chino/os/kernel/kd.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::kernel::io;
using namespace chino::os::kernel::ps;

result<void> device::close([[maybe_unused]] file &file) noexcept { return ok(); }

result<void> device::fast_open([[maybe_unused]] file &file, std::string_view path,
                               [[maybe_unused]] create_disposition create_disposition) noexcept {
    if (path.empty()) {
        return ok();
    }
    return err(error_code::invalid_path);
}

result<size_t> device::fast_read([[maybe_unused]] file &file, [[maybe_unused]] std::span<std::byte> buffer,
                                 [[maybe_unused]] std::optional<size_t> offset) noexcept {
    return err(error_code::slow_io);
}

result<size_t> device::fast_write([[maybe_unused]] file &file, [[maybe_unused]] std::span<const std::byte> buffer,
                                  [[maybe_unused]] std::optional<size_t> offset) noexcept {
    return err(error_code::slow_io);
}

result<size_t> device::fast_control([[maybe_unused]] file &file, [[maybe_unused]] control_code_t code,
                                    [[maybe_unused]] void *arg) noexcept {
    return err(error_code::slow_io);
}

void device::queue_io(io_request &irp) noexcept {
    std::unique_lock<irq_spin_lock> lock(irps_lock_);
    irps_.push_back(&irp);
    io::register_device_process_io(*this);
}

bool device::process_queued_ios(io_request *wait_irp) noexcept {
    kassert(!hal::arch_t::in_irq_handler());
    while (true) {
        io_request *irp;
        {
            std::unique_lock<ps::irq_spin_lock> lock(irps_lock_);
            irp = irps_.front();
            if (irp) {
                irps_.remove(irp);
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
                if (irp->is_completed()) {
                    // IRP is completed
                    irp->dec_ref();
                    if (irp == wait_irp) {
                        return true;
                    } else {
                        continue;
                    }
                }
            } else {
                queue_pending_io(*irp);
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

result<void> device::cancel_io([[maybe_unused]] io_request &irp) noexcept { return err(error_code::not_supported); }

void device::on_io_completion(io_request &irp) noexcept { irp.complete(); }

void device::requeue_pending_io(bool (*pred)(io_request &, void *), void *arg) noexcept {
    io_request *irp;
    {
        std::unique_lock<ps::irq_spin_lock> lock(irps_lock_);
        irp = pending_irps_.front();
        while (irp) {
            if (!pred || pred(*irp, arg)) {
                pending_irps_.remove(irp);
                break;
            }
            irp = pending_irps_.next(irp);
        }
    }
    kassert(irp);
    queue_io(*irp);
}

void device::requeue_pending_io(io_frame_kind kind) noexcept {
    requeue_pending_io(
        [](io_request &irp, void *arg) {
            return irp.current_frame().unwrap()->kind() == *reinterpret_cast<io_frame_kind *>(arg);
        },
        &kind);
}

void device::queue_pending_io(io_request &irp) noexcept {
    std::unique_lock<ps::irq_spin_lock> lock(irps_lock_);
    pending_irps_.push_back(&irp);
}
