// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "host_console.h"
#include <chino/os/kernel/io.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::drivers;

namespace {
HRESULT PrepareStartupInformation(HPCON hpc, STARTUPINFOEXA *psi) {
    // Prepare Startup Information structure
    STARTUPINFOEXA si;
    ZeroMemory(&si, sizeof(si));
    si.StartupInfo.cb = sizeof(STARTUPINFOEXA);

    // Discover the size required for the list
    size_t bytesRequired;
    InitializeProcThreadAttributeList(NULL, 1, 0, &bytesRequired);

    // Allocate memory to represent the list
    si.lpAttributeList = (PPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(GetProcessHeap(), 0, bytesRequired);
    if (!si.lpAttributeList) {
        return E_OUTOFMEMORY;
    }

    // Initialize the list memory location
    if (!InitializeProcThreadAttributeList(si.lpAttributeList, 1, 0, &bytesRequired)) {
        HeapFree(GetProcessHeap(), 0, si.lpAttributeList);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Set the pseudoconsole information into the list
    if (!UpdateProcThreadAttribute(si.lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE, hpc, sizeof(hpc), NULL,
                                   NULL)) {
        HeapFree(GetProcessHeap(), 0, si.lpAttributeList);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    *psi = si;

    return S_OK;
}
} // namespace

HRESULT host_console_device::SetUpPseudoConsole(COORD size) noexcept {
    HRESULT hr = S_OK;

    // Create communication channels

    // - Close these after CreateProcess of child application with pseudoconsole object.
    HANDLE inputReadSide, outputWriteSide;

    // - Hold onto these and use them for communication with the child through the pseudoconsole.
    HANDLE outputReadSide, inputWriteSide;

    if (!CreatePipe(&inputReadSide, &inputWriteSide, NULL, 0)) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (!CreatePipe(&outputReadSide, &outputWriteSide, NULL, 0)) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    HPCON hPC;
    hr = CreatePseudoConsole(size, inputReadSide, outputWriteSide, 0, &hPC);
    if (FAILED(hr)) {
        return hr;
    }

    STARTUPINFOEXA siEx;
    hr = PrepareStartupInformation(hPC, &siEx);
    if (FAILED(hr)) {
        return hr;
    }

    PCSTR childApplication = "C:\\windows\\system32\\cmd.exe";

    // Create mutable text string for CreateProcessW command line string.
    const size_t charsRequired = strlen(childApplication) + 1; // +1 null terminator
    PSTR cmdLineMutable = (PSTR)HeapAlloc(GetProcessHeap(), 0, sizeof(char) * charsRequired);

    if (!cmdLineMutable) {
        return E_OUTOFMEMORY;
    }

    strcpy(cmdLineMutable, childApplication);

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    // Call CreateProcess
    if (!CreateProcessA(NULL, cmdLineMutable, NULL, NULL, FALSE, EXTENDED_STARTUPINFO_PRESENT, NULL, NULL,
                        &siEx.StartupInfo, &pi)) {
        HeapFree(GetProcessHeap(), 0, cmdLineMutable);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    stdin_ = inputReadSide;
    stdout_ = outputReadSide;
    return S_OK;
}

result<void> host_console_device::install() noexcept {
    // AllocConsole();
    // SetUpPseudoConsole({.X = 320, .Y = 320});
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
        return ok(file(*this));
    }
    return err(error_code::invalid_path);
}

result<void> host_console_device::close(file &file) noexcept { return ok(); }

result<size_t> host_console_device::read(file &file, std::span<const iovec> iovs, size_t offset) noexcept {
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
    return ok(total_read);
}

result<size_t> host_console_device::write(file &file, std::span<const iovec> iovs, size_t offset) noexcept {
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
