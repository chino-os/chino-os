// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "host_console.h"
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

result<file> host_console_device::open(std::string_view path, create_disposition disposition) noexcept {
    if (path.empty()) {
        return ok(file(*this, access_mask::generic_all));
    }
    return err(error_code::invalid_path);
}

result<void> host_console_device::close(file &file) noexcept { return ok(); }

result<size_t> host_console_device::read(file &file, std::span<const iovec> iovs,
                                         std::optional<size_t> /*offset*/) noexcept {
    size_t total_read = 0;
    for (auto iov : iovs) {
        DWORD to_read = iov.iov_len;
        char *iov_base = reinterpret_cast<char *>(iov.iov_base);
        while (to_read) {
            DWORD unread_events;
            {
                ps::current_irq_lock irq_lock;
                GetNumberOfConsoleInputEvents(stdin_, &unread_events);
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
                        if (!RegisterWaitForSingleObject(&stdin_wait_handle_, stdin_, handle_ready_callback, nullptr,
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
                    if (!ReadConsoleInputA(stdin_, &rec, 1, &events_read)) {
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

result<size_t> host_console_device::write(file &file, std::span<const iovec> iovs,
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

result<size_t> host_console_device::control(file &file, control_code_t code, std::span<const std::byte> in_buffer,
                                            std::span<std::byte> out_buffer) noexcept {
    return err(error_code::not_supported);
}

result<void> host_console_device::stdin_irq_handler(hal::arch_irq_number_t, void *context) noexcept {
    auto *device = reinterpret_cast<host_console_device *>(context);
    device->stdin_avail_event_.notify_all();
    return ok();
}

result<void> host_console_driver::install_device(host_console_device &device) noexcept {
    try_(device.install());
    try_(io::attach_device(device));
    return ok();
}
