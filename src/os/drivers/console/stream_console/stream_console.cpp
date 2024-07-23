// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "stream_console.h"
#include <chino/os/kernel/io.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::drivers;

result<void> stream_console_device::install(device &stream_device) noexcept {
    try_set(stream_file_, io::open_file(stream_device, {}, create_disposition::open_existing));
    return ok();
}

result<file> stream_console_device::open(std::string_view path, create_disposition create_disposition) noexcept {
    if (path.empty()) {
        return ok(file(*this, access_mask::generic_all));
    }
    return err(error_code::invalid_path);
}

result<void> stream_console_device::close(file &file) noexcept { return ok(); }

result<size_t> stream_console_device::read(file &file, std::span<const iovec> iovs,
                                           std::optional<size_t> /*offset*/) noexcept {
    return io::read_file(stream_file_, iovs);
}

result<size_t> stream_console_device::write(file &file, std::span<const iovec> iovs,
                                            std::optional<size_t> /*offset*/) noexcept {
    return io::write_file(stream_file_, iovs);
}

result<int> stream_console_device::control(file &file, int request, va_list ap) noexcept {
    return io::control_file(stream_file_, request, ap);
}

result<void> stream_console_driver::install_device(stream_console_device &device,
                                                   chino::os::device &stream_device) noexcept {
    try_(device.install(stream_device));
    try_(io::attach_device(device));
    return ok();
}
