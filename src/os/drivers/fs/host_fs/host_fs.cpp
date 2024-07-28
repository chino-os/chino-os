// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "host_fs.h"
#include "../../../hal/archs/emulator/emulator_cpu.h"
#include <Shlwapi.h>
#include <chino/os/kernel/io.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::kernel::io;
using namespace chino::os::drivers;

result<void> host_fs_device::install() noexcept {
    GetModuleFileNameA(hal::hal_instance, base_dirname_, MAX_PATH);
    PathCombineA(base_dirname_, base_dirname_, "..\\..\\");
    return ok();
}

result<void> host_fs_device::fast_open(file &file, std::string_view path, create_disposition disposition) noexcept {
    if (!path.empty()) {
        char host_filename[MAX_PATH];
        PathCombineA(host_filename, base_dirname_, path.data());
        auto handle =
            CreateFileA(host_filename, GENERIC_READ | GENERIC_WRITE, 0, nullptr, (DWORD)disposition, 0, nullptr);
        if (handle != INVALID_HANDLE_VALUE) {
            file.data(handle);
            return ok();
        } else {
            return err(error_code::not_found);
        }
    }
    return err(error_code::invalid_path);
}

result<void> host_fs_device::close(file &file) noexcept {
    return CloseHandle(file.data<HANDLE>()) ? ok() : err(error_code::io_error);
}

result<size_t> host_fs_device::fast_read(file &file, std::span<std::byte> buffer,
                                         std::optional<size_t> offset) noexcept {
    DWORD bytes_read;
    TRY_WIN32_IO_IF_NOT(ReadFile(file.data<HANDLE>(), buffer.data(), buffer.size_bytes(), &bytes_read, nullptr));
    return ok(bytes_read);
}

result<size_t> host_fs_device::fast_write(file &file, std::span<const std::byte> buffer,
                                          std::optional<size_t> offset) noexcept {
    DWORD bytes_written;
    TRY_WIN32_IO_IF_NOT(WriteFile(file.data<HANDLE>(), buffer.data(), buffer.size_bytes(), &bytes_written, nullptr));
    return ok(bytes_written);
}

result<void> host_fs_driver::install_device(host_fs_device &device) noexcept {
    try_(device.install());
    try_(io::attach_device(device));
    return ok();
}
