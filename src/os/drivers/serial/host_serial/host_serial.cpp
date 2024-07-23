// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "host_serial.h"
#include <chino/os/kernel/io.h>
#include <chino/os/kernel/ps.h>
#include <termios.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::drivers;

namespace {
void WINAPI port_io_completed(_In_ DWORD dwErrorCode, _In_ DWORD dwNumberOfBytesTransfered,
                              _Inout_ LPOVERLAPPED lpOverlapped) {
    auto *overlapped = static_cast<hal::irq_overlapped *>(lpOverlapped);
    hal::arch_t::send_irq(overlapped->number);
}
} // namespace

result<void> host_serial_device::install(std::string_view port_name) noexcept {
    port_ = CreateFileA(port_name.data(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED,
                        nullptr);
    if (port_ == INVALID_HANDLE_VALUE) {
        return err(error_code::not_found);
    }
    CreateIoCompletionPort(port_, hal::emulator::iocp_port(), 0, 0);
    BindIoCompletionCallback(port_, port_io_completed, 0);
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
        hal::irq_overlapped overlapped{.number = hal::arch_irq_number_t::host_serial_rx};
        {
            ps::current_irq_lock irq_lock;
            if (!ReadFile(port_, iov.iov_base, iov.iov_len, nullptr, &overlapped)) {
                return err(error_code::io_error);
            }
        }
    }
    return ok(total_read);
}

result<size_t> host_serial_device::write(file &file, std::span<const iovec> iovs,
                                         std::optional<size_t> /*offset*/) noexcept {
    size_t total_written = 0;
    for (auto iov : iovs) {
        hal::irq_overlapped overlapped{.number = hal::arch_irq_number_t::host_serial_rx};
        {
            ps::current_irq_lock irq_lock;
            if (!WriteFile(port_, iov.iov_base, iov.iov_len, nullptr, &overlapped)) {
                if (GetLastError() != ERROR_IO_PENDING) {
                    return err(error_code::io_error);
                }
            }
        }
    }
    return ok(total_written);
}

result<int> host_serial_device::control(file &file, int request, va_list ap) noexcept {
    switch (request) {
    case TCGETS: {
        auto arg = va_arg(ap, termios *);
        if (!arg) {
            return err(error_code::argument_null);
        }
        arg->c_iflag = 0;
        break;
    }
    default:
        break;
    }
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
