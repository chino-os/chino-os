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
constinit ps::event stdin_avail_event_;
std::array<INPUT_RECORD, 256> stdin_records_;

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
#if 1
    size_t total_read = 0;
    for (auto iov : iovs) {
        DWORD to_read = iov.iov_len;
        char *iov_base = reinterpret_cast<char *>(iov.iov_base);
        while (true) {
            DWORD unread_events;
            GetNumberOfConsoleInputEvents(stdin_, &unread_events);
            if (!unread_events) {
                if (!total_read) {
                    HANDLE wait_handle;
                    if (!RegisterWaitForSingleObject(&wait_handle, stdin_, handle_ready_callback, nullptr, INFINITE,
                                                     WT_EXECUTEONLYONCE)) {
                        return err(error_code::io_error);
                    }
                    try_(stdin_avail_event_.wait());
                } else {
                    return ok(total_read);
                }
            } else {
                DWORD events_read = std::min({unread_events, to_read, (DWORD)stdin_records_.size()});
                if (!ReadConsoleInputA(stdin_, stdin_records_.data(), events_read, &events_read)) {
                    return err(error_code::io_error);
                }
                for (size_t i = 0; i < events_read; i++) {
                    auto &record = stdin_records_[i];
                    switch (record.EventType) {
                    case KEY_EVENT: {
                        auto &k_record = reinterpret_cast<KEY_EVENT_RECORD &>(record);
                        *iov_base++ = k_record.uChar.AsciiChar;
                        to_read--;
                        total_read++;
                        break;
                    }
                    default:
                        break;
                    }
                }
            }
        }
    }
#else
    size_t total_read = 0;
    for (auto iov : iovs) {
        DWORD read;
        if (!ReadConsoleA(stdin_, iov.iov_base, iov.iov_len, &read, nullptr)) {
            return err(error_code::io_error);
        }

        // interpret \r to \n
        for (size_t i = 0; i < read; i++) {
            auto &c = *(reinterpret_cast<char *>(iov.iov_base) + i);
            if (c == '\r')
                c = '\n';
            else if (c == 0x7f)
                c = '\b';
        }

        if (!read)
            break;
        total_read += read;
    }
#endif
    return ok(total_read);
}

result<size_t> host_console_device::write(file &file, std::span<const iovec> iovs,
                                          std::optional<size_t> /*offset*/) noexcept {
    size_t total_written = 0;
    for (auto iov : iovs) {
        DWORD written;
        WriteConsoleA(stdout_, iov.iov_base, iov.iov_len, &written, nullptr);
        total_written += written;
    }
    return ok(total_written);
}

result<size_t> host_console_device::control(file &file, control_code_t code, std::span<const std::byte> in_buffer,
                                            std::span<std::byte> out_buffer) noexcept {
    return err(error_code::not_supported);
}

result<void> host_console_driver::install_device(host_console_device &device) noexcept {
    try_(device.install());
    try_(io::attach_device(device));
    return ok();
}
