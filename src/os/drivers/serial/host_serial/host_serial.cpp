// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "host_serial.h"
#include <chino/os/kernel/io.h>
#include <chino/os/kernel/ps.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::drivers;

namespace {
void CALLBACK handle_ready_callback(_In_ PVOID lpParameter, _In_ BOOLEAN TimerOrWaitFired) {
    hal::arch_t::send_irq(hal::arch_irq_number_t::host_console_stdin);
}
} // namespace

result<void> host_serial_device::install(std::string_view port_name) noexcept {
    port_ = CreateFileA(port_name.data(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (port_ == INVALID_HANDLE_VALUE) {
        return err(error_code::not_found);
    }
    try_(io::register_irq_handler(hal::arch_irq_number_t::host_console_stdin, stdin_irq_handler, this));
    return ok();
}

result<file> host_serial_device::open(std::string_view path, create_disposition disposition) noexcept {
    if (path.empty()) {
        return ok(file(*this, access_mask::generic_all));
    }
    return err(error_code::invalid_path);
}

result<void> host_serial_device::close(file &file) noexcept { return ok(); }

result<size_t> host_serial_device::read(file &file, std::span<const iovec> iovs,
                                        std::optional<size_t> /*offset*/) noexcept {
    size_t total_read = 0;
    for (auto iov : iovs) {
        DWORD to_read = iov.iov_len;
        char *iov_base = reinterpret_cast<char *>(iov.iov_base);
        while (to_read) {
            DWORD unread_events;
            {
                ps::current_irq_lock irq_lock;
                GetNumberOfConsoleInputEvents(port_, &unread_events);
            }
            if (!unread_events) {
                if (!total_read) {
                    {
                        ps::current_irq_lock irq_lock;
                        auto wait_handle = stdin_wait_handle_;
                        if (wait_handle) {
                            UnregisterWait(wait_handle);
                            stdin_wait_handle_ = nullptr;
                        }
                        if (!RegisterWaitForSingleObject(&stdin_wait_handle_, port_, handle_ready_callback, nullptr,
                                                         INFINITE, WT_EXECUTEONLYONCE | WT_EXECUTEINWAITTHREAD)) {
                            return err(error_code::io_error);
                        }
                    }
                    try_(stdin_avail_event_.wait());
                } else {
                    return ok(total_read);
                }
            } else {
                INPUT_RECORD rec;
                DWORD events_read;
                {
                    ps::current_irq_lock irq_lock;
                    if (!ReadConsoleInputA(port_, &rec, 1, &events_read)) {
                        return err(error_code::io_error);
                    }
                }
                if (rec.EventType != KEY_EVENT || !rec.Event.KeyEvent.bKeyDown) { // only want key down events
                    continue;
                }
                char c = rec.Event.KeyEvent.uChar.AsciiChar;
                if (c) {
                    *iov_base++ = c;
                    to_read--;
                    total_read++;
                }
            }
        }
    }
    return ok(total_read);
}

result<size_t> host_serial_device::write(file &file, std::span<const iovec> iovs,
                                         std::optional<size_t> /*offset*/) noexcept {
    size_t total_written = 0;
    for (auto iov : iovs) {
        DWORD written;
        ps::current_irq_lock irq_lock;
        WriteConsoleA(stdout_, iov.iov_base, iov.iov_len, &written, nullptr);
        total_written += written;
    }
    return ok(total_written);
}

result<size_t> host_serial_device::control(file &file, control_code_t code, std::span<const std::byte> in_buffer,
                                           std::span<std::byte> out_buffer) noexcept {
    return err(error_code::not_supported);
}

result<void> host_serial_device::stdin_irq_handler(hal::arch_irq_number_t, void *context) noexcept {
    auto *device = reinterpret_cast<host_serial_device *>(context);
    device->stdin_avail_event_.notify_all();
    return ok();
}

result<void> host_serial_driver::install_device(host_serial_device &device) noexcept {
    try_(device.install(hal::meta::board_desc::chip::machine::devices::host_serial::properties::port_name));
    try_(io::attach_device(device));
    return ok();
}
