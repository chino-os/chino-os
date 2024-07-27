// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "host_serial.h"
#include <chino/bits.h>
#include <chino/os/kernel/io.h>
#include <chino/os/kernel/ps.h>
#include <termios.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::kernel::io;
using namespace chino::os::drivers;

namespace {
void WINAPI port_io_completed(_In_ DWORD dwErrorCode, _In_ DWORD dwNumberOfBytesTransfered,
                              _Inout_ LPOVERLAPPED lpOverlapped) {
    auto *overlapped = static_cast<hal::irq_overlapped *>(lpOverlapped);
    overlapped->bytes_transferred = dwNumberOfBytesTransfered;
    hal::arch_t::send_irq(overlapped->number);
}

void get_parity(termios *term, const DCB &dcb) noexcept {
    set_bits(term->c_cflag, PARENB, dcb.fParity);
    set_bits(term->c_cflag, PARODD, dcb.Parity == 1);
}

void set_parity(DCB &dcb, const termios *term) noexcept {
    if (term->c_cflag & PARENB) {
        dcb.fParity = TRUE;
        if (term->c_cflag & PARODD)
            dcb.Parity = ODDPARITY;
        else
            dcb.Parity = EVENPARITY;
    } else {
        dcb.fParity = FALSE;
        dcb.Parity = NOPARITY;
    }
}

void get_stopbits(termios *term, const DCB &dcb) noexcept { set_bits(term->c_cflag, CSTOPB, dcb.StopBits == 2); }

void set_stopbits(DCB &dcb, const termios *term) noexcept {
    if (term->c_cflag & CSTOPB)
        dcb.StopBits = TWOSTOPBITS;
    else
        dcb.StopBits = ONESTOPBIT;
}

void get_bytesize(termios *term, const DCB &dcb) noexcept {
    switch (dcb.ByteSize) {
    case 5:
        term->c_cflag |= CS5;
        break;
    case 6:
        term->c_cflag |= CS6;
        break;
    case 7:
        term->c_cflag |= CS7;
        break;
    case 8:
        term->c_cflag |= CS8;
        break;
    }
}

void set_bytesize(DCB &dcb, const termios *term) noexcept {
    switch (term->c_cflag & CSIZE) {
    case CS5:
        dcb.ByteSize = 5;
        break;
    case CS6:
        dcb.ByteSize = 6;
        break;
    case CS7:
        dcb.ByteSize = 7;
        break;
    case CS8:
        dcb.ByteSize = 8;
        break;
    }
}

void get_timeouts(termios *term, const COMMTIMEOUTS &timeouts) noexcept {
    if (timeouts.ReadIntervalTimeout == MAXDWORD) {
        if (timeouts.ReadTotalTimeoutMultiplier == MAXDWORD) {
            // 1. Blocking wait for any data
            term->c_cc[VMIN] = 0;
            term->c_cc[VTIME] = timeouts.ReadTotalTimeoutConstant / 100; // in 100ms
        } else {
            // 2. Non-blocking
            term->c_cc[VMIN] = 0;
            term->c_cc[VTIME] = 0;
        }
    } else if (!timeouts.ReadIntervalTimeout && !timeouts.ReadTotalTimeoutMultiplier &&
               !timeouts.ReadTotalTimeoutConstant) {
        // 3. Blocking forever
        term->c_cc[VMIN] = 1;
        term->c_cc[VTIME] = 0;
    } else {
        term->c_cc[VMIN] = 1;
        term->c_cc[VTIME] = timeouts.ReadIntervalTimeout / 100; // interval
    }
}

void set_timeouts(COMMTIMEOUTS &timeouts, const termios *term) noexcept {
    timeouts.WriteTotalTimeoutConstant = 0;
    timeouts.WriteTotalTimeoutMultiplier = 0;

    if (!term->c_cc[VMIN]) {
        if (term->c_cc[VTIME]) {
            // 1. Blocking wait for any data
            timeouts.ReadIntervalTimeout = MAXDWORD;
            timeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
            timeouts.ReadTotalTimeoutConstant = term->c_cc[VTIME] * 100; // in ms
        } else {
            // 2. Non-blocking
            timeouts.ReadIntervalTimeout = MAXDWORD;
            timeouts.ReadTotalTimeoutMultiplier = 0;
            timeouts.ReadTotalTimeoutConstant = 0;
        }
    } else if (!term->c_cc[VTIME]) {
        // 3. Blocking forever
        timeouts.ReadIntervalTimeout = 0;
        timeouts.ReadTotalTimeoutMultiplier = 0;
        timeouts.ReadTotalTimeoutConstant = 0;
    } else {
        timeouts.ReadIntervalTimeout = term->c_cc[VTIME] * 100; // interval
        timeouts.ReadTotalTimeoutMultiplier = 0;
        timeouts.ReadTotalTimeoutConstant = 0;
    }
}
} // namespace

result<void> host_serial_device::install(std::string_view port_name) noexcept {
    TRY_WIN32_IF_NOT((port_ = CreateFileA(port_name.data(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING,
                                          FILE_FLAG_OVERLAPPED, nullptr)) != INVALID_HANDLE_VALUE);
    TRY_WIN32_IF_NOT(BindIoCompletionCallback(port_, port_io_completed, 0));
    try_(io::register_irq_handler(hal::arch_irq_number_t::host_serial_rx, rx_irq_handler, this));
    try_(io::register_irq_handler(hal::arch_irq_number_t::host_serial_tx, tx_irq_handler, this));
    return ok();
}

result<void> host_serial_device::process_io(io_request &irp) noexcept {
    try_var(frame, irp.current_frame());
    if (frame->kind().major == io_frame_major_kind::generic) {
        auto &params = frame->params<io_frame_params_generic>();
        switch ((io_frame_generic_kind)frame->kind().minor) {
        case io_frame_generic_kind::read: {
            rx_overlapped_ = {.number = hal::arch_irq_number_t::host_serial_rx};
            TRY_WIN32_IO_IF_NOT(ReadFile(port_, params.read.buffer.data(), params.read.buffer.size_bytes(),
                                         &rx_overlapped_.bytes_transferred, &rx_overlapped_));
            break;
        }
        case io_frame_generic_kind::write: {
            tx_overlapped_ = {.number = hal::arch_irq_number_t::host_serial_tx};
            TRY_WIN32_IO_IF_NOT(WriteFile(port_, params.write.buffer.data(), params.write.buffer.size_bytes(),
                                          &tx_overlapped_.bytes_transferred, &tx_overlapped_));
            break;
        }
        default:
            break;
        }
    }
    return device::process_io(irp);
}

result<size_t> host_serial_device::fast_control(file &file, control_code_t code, void *arg) noexcept {
    switch (code) {
    case TCGETS: {
        auto term = reinterpret_cast<termios *>(arg);
        if (!term) {
            return err(error_code::argument_null);
        }
        *term = {};

        DCB dcb{.DCBlength = sizeof(DCB)};
        TRY_WIN32_IF_NOT(GetCommState(port_, &dcb));
        get_parity(term, dcb);
        get_stopbits(term, dcb);
        get_bytesize(term, dcb);
        set_bits(term->c_cflag, CCTS_OFLOW, dcb.fOutxCtsFlow);
        set_bits(term->c_cflag, CRTS_IFLOW, dcb.fRtsControl);
        term->c_speed = dcb.BaudRate;

        COMMTIMEOUTS timeouts;
        TRY_WIN32_IF_NOT(GetCommTimeouts(port_, &timeouts));
        get_timeouts(term, timeouts);
        return ok(0);
    }
    case TCSETS: {
        auto term = reinterpret_cast<termios *>(arg);
        if (!term) {
            return err(error_code::argument_null);
        }

        DCB dcb{.DCBlength = sizeof(DCB)};
        TRY_WIN32_IF_NOT(GetCommState(port_, &dcb));
        set_parity(dcb, term);
        set_stopbits(dcb, term);
        set_bytesize(dcb, term);
        dcb.fOutxCtsFlow = term->c_cflag & CCTS_OFLOW;
        dcb.fRtsControl = term->c_cflag & CCTS_OFLOW;
        dcb.BaudRate = term->c_speed;
        dcb.fDtrControl = DTR_CONTROL_DISABLE;
        dcb.fInX = FALSE;
        dcb.fOutX = FALSE;
        dcb.fDsrSensitivity = FALSE;
        TRY_WIN32_IF_NOT(SetCommState(port_, &dcb));

        COMMTIMEOUTS timeouts;
        set_timeouts(timeouts, term);
        TRY_WIN32_IF_NOT(SetCommTimeouts(port_, &timeouts));
        return ok(0);
    }
    case TCFLSH: {
        const auto src_flags = reinterpret_cast<uintptr_t>(arg);
        DWORD flags = 0;
        if (src_flags == TCIFLUSH)
            flags |= PURGE_RXCLEAR;
        else if (src_flags == TCOFLUSH)
            flags |= PURGE_TXCLEAR;
        else if (src_flags == TCIOFLUSH)
            flags |= PURGE_RXCLEAR | PURGE_TXCLEAR;
        else
            return err(error_code::invalid_argument);
        TRY_WIN32_IF_NOT(PurgeComm(port_, flags));
        return ok(0);
    }
    case TCDRN: {
        TRY_WIN32_IF_NOT(FlushFileBuffers(port_));
        return ok(0);
    }
    default:
        break;
    }
    return err(error_code::not_supported);
}

result<void> host_serial_device::rx_irq_handler(hal::arch_irq_number_t, void *context) noexcept {
    auto *device = reinterpret_cast<host_serial_device *>(context);
    auto irp = device->current_irp();
    irp->complete(ok(device->rx_overlapped_.bytes_transferred));
    return ok();
}

result<void> host_serial_device::tx_irq_handler(hal::arch_irq_number_t, void *context) noexcept {
    auto *device = reinterpret_cast<host_serial_device *>(context);
    auto irp = device->current_irp();
    irp->complete(ok(device->tx_overlapped_.bytes_transferred));
    return ok();
}

result<void> host_serial_driver::install_device(host_serial_device &device) noexcept {
    try_(device.install(hal::meta::board_desc::chip::machine::devices::host_serial::properties::port_name));
    try_(io::attach_device(device));
    return ok();
}
