// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "host_console.h"
#include <chino/os/kernel/io.h>
#include <chino/os/kernel/ps.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::kernel::io;
using namespace chino::os::drivers;

namespace {
void CALLBACK handle_ready_callback(_In_ PVOID lpParameter, _In_ BOOLEAN TimerOrWaitFired) {
    hal::arch_t::send_irq(hal::arch_irq_number_t::host_console_stdin);
}
} // namespace

result<void> host_console_device::install() noexcept {
    AllocConsole();
    SetConsoleTitle(L"Chino Terminal");
    stdin_ = GetStdHandle(STD_INPUT_HANDLE);
    stdout_ = GetStdHandle(STD_OUTPUT_HANDLE);

    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    SetConsoleMode(stdin_, ENABLE_INSERT_MODE | ENABLE_QUICK_EDIT_MODE | ENABLE_VIRTUAL_TERMINAL_INPUT);
    SetConsoleMode(stdout_, ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    CONSOLE_FONT_INFOEX font_info{};
    font_info.cbSize = sizeof(font_info);
    font_info.dwFontSize = {0, 20};
    font_info.FontWeight = FW_NORMAL;
    lstrcpy(font_info.FaceName, L"Cascadia Mono");
    SetCurrentConsoleFontEx(stdout_, FALSE, &font_info);
    try_(io::register_irq_handler(hal::arch_irq_number_t::host_console_stdin, stdin_irq_handler, this));
    return ok();
}

result<size_t> host_console_device::fast_read(file &file, std::span<std::byte> buffer,
                                              std::optional<size_t> /*offset*/) noexcept {
    if (buffer.empty())
        return err(error_code::insufficient_buffer);

    size_t bytes_read = 0;
    while (!buffer.empty()) {
        DWORD unread_events;
        TRY_WIN32_IF_NOT(GetNumberOfConsoleInputEvents(stdin_, &unread_events));
        if (!unread_events)
            break;

        while (unread_events--) {
            INPUT_RECORD rec;
            DWORD events_read;
            TRY_WIN32_IF_NOT(ReadConsoleInputA(stdin_, &rec, 1, &events_read));
            if (rec.EventType != KEY_EVENT || !rec.Event.KeyEvent.bKeyDown) { // only want key down events
                continue;
            }

            char c = rec.Event.KeyEvent.uChar.AsciiChar;
            if (c) {
                if (c == 0x7f)
                    c = '\b';
                buffer[0] = (std::byte)c;
                buffer = buffer.subspan(1);
                bytes_read++;
                if (buffer.empty())
                    break;
            }
        }
    }
    return bytes_read ? ok(bytes_read) : err(error_code::slow_io);
}

result<void> host_console_device::process_io(io_request &irp) noexcept {
    try_var(frame, irp.current_frame());
    if (frame->kind().major == io_frame_major_kind::generic) {
        auto &params = frame->params<io_frame_params_generic>();
        switch ((io_frame_generic_kind)frame->kind().minor) {
        case io_frame_generic_kind::read: {
            auto wait_handle = stdin_wait_handle_;
            if (wait_handle) {
                TRY_WIN32_IF_NOT(UnregisterWait(wait_handle));
                stdin_wait_handle_ = nullptr;
            }

            auto r = fast_read(frame->file(), params.read.buffer, std::nullopt);
            if (r.is_ok()) {
                irp.complete(std::move(r));
                return ok();
            }

            TRY_WIN32_IF_NOT(RegisterWaitForSingleObject(&stdin_wait_handle_, stdin_, handle_ready_callback, nullptr,
                                                         INFINITE, WT_EXECUTEONLYONCE | WT_EXECUTEINWAITTHREAD));
            return ok();
        }
        default:
            break;
        }
    }
    return device::process_io(irp);
}

result<size_t> host_console_device::fast_write(file &file, std::span<const std::byte> buffer,
                                               std::optional<size_t> offset) noexcept {
    DWORD bytes_written;
    TRY_WIN32_IF_NOT(WriteConsoleA(stdout_, buffer.data(), buffer.size_bytes(), &bytes_written, nullptr));
    return ok(bytes_written);
}

result<void> host_console_device::stdin_irq_handler(hal::arch_irq_number_t, void *context) noexcept {
    auto *device = reinterpret_cast<host_console_device *>(context);
    auto irp = device->current_irp();
    return irp->queue();
}

result<void> host_console_driver::install_device(host_console_device &device) noexcept {
    try_(device.install());
    try_(io::attach_device(device));
    return ok();
}
