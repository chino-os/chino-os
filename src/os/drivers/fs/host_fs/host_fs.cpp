// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "host_fs.h"
#include "../../../hal/archs/emulator/emulator_cpu.h"
#include <Shlwapi.h>
#include <chino/os/kernel/io.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::drivers;

result<void> host_fs_device::install() noexcept {
    GetModuleFileNameA(hal::hal_instance, base_dirname_, MAX_PATH);
    PathCombineA(base_dirname_, base_dirname_, "..\\..\\");
    return ok();
}

result<file> host_fs_device::open(std::string_view path, create_disposition disposition) noexcept {
    if (!path.empty()) {
        char host_filename[MAX_PATH];
        PathCombineA(host_filename, base_dirname_, path.data());
        auto handle =
            CreateFileA(host_filename, GENERIC_READ | GENERIC_WRITE, 0, nullptr, (DWORD)disposition, 0, nullptr);
        if (handle != INVALID_HANDLE_VALUE) {
            return ok(file(*this, access_mask::generic_all, handle));
        } else {
            return err(error_code::not_found);
        }
    }
    return err(error_code::invalid_path);
}

result<void> host_fs_device::close(file &file) noexcept {
    return CloseHandle(file.data<HANDLE>()) ? ok() : err(error_code::io_error);
}

result<size_t> host_fs_device::read(file &file, std::span<const iovec> iovs, std::optional<size_t> offset) noexcept {
    size_t total_read = 0;
    for (auto iov : iovs) {
        DWORD read;
        if (!ReadFile(file.data<HANDLE>(), iov.iov_base, iov.iov_len, &read, nullptr)) {
            return err(error_code::io_error);
        }
        if (!read)
            break;
        total_read += read;
    }
    return ok(total_read);
}

result<size_t> host_fs_device::write(file &file, std::span<const iovec> iovs, std::optional<size_t> offset) noexcept {
    size_t total_written = 0;
    for (auto iov : iovs) {
        DWORD written;
        if (!WriteFile(file.data<HANDLE>(), iov.iov_base, iov.iov_len, &written, nullptr)) {
            return err(error_code::io_error);
        }
        total_written += written;
    }
    return ok(total_written);
}

result<int> host_fs_device::control(file &file, int request, void *arg) noexcept {
    return err(error_code::not_supported);
}

result<void> host_fs_driver::install_device(host_fs_device &device) noexcept {
    try_(device.install());
    try_(io::attach_device(device));
    return ok();
}
