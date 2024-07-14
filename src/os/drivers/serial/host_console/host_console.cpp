// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "host_console.h"
#include <chino/os/kernel/io.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::drivers;

namespace {
constinit static_object<file> file_;
} // namespace

result<void> host_console_device::install() noexcept {
    SetConsoleTitle(L"Chino Terminal");
    stdin_ = GetStdHandle(STD_INPUT_HANDLE);
    stdout_ = GetStdHandle(STD_OUTPUT_HANDLE);

    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    SetConsoleMode(stdin_, ENABLE_INSERT_MODE | ENABLE_QUICK_EDIT_MODE | ENABLE_VIRTUAL_TERMINAL_INPUT);
    SetConsoleMode(stdout_, ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    CONSOLE_FONT_INFOEX font_info{};
    font_info.cbSize = sizeof(font_info);
    font_info.dwFontSize = {0, 18};
    font_info.FontWeight = FW_NORMAL;
    lstrcpy(font_info.FaceName, L"Consolas");
    SetCurrentConsoleFontEx(stdout_, FALSE, &font_info);
    file_.initialize(*this);
    return ok();
}

result<object_ptr<file>> host_console_device::open(std::string_view path,
                                                   create_disposition create_disposition) noexcept {
    if (path.empty()) {
        return ok(file_.get());
    }
    return err(error_code::invalid_path);
}

result<void> host_console_device::close(file &file) noexcept { return ok(); }

result<size_t> host_console_device::read(file &file, std::span<const iovec> iovs, size_t offset) noexcept {
    return ok(1);
}

result<size_t> host_console_device::write(file &file, std::span<const iovec> iovs, size_t offset) noexcept {
    return ok(1);
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
