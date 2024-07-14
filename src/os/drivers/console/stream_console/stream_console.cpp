// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "stream_console.h"
#include <chino/os/kernel/io.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::drivers;

namespace {
constinit static_object<file> file_;
} // namespace

result<void> stream_console_device::install(device &stream_device) noexcept {
    try_var(stream_file_, io::open_file(stream_device, {}, create_disposition::open_existing));
    file_.initialize(*this);
    return ok();
}

result<object_ptr<file>> stream_console_device::open(std::string_view path,
                                                     create_disposition create_disposition) noexcept {
    if (path.empty()) {
        return ok(file_.get());
    }
    return err(error_code::invalid_path);
}

result<void> stream_console_device::close(file &file) noexcept { return ok(); }

result<size_t> stream_console_device::read(file &file, std::span<const iovec> iovs, size_t /*offset*/) noexcept {
    return io::read_file(file, iovs);
}

result<size_t> stream_console_device::write(file &file, std::span<const iovec> iovs, size_t /*offset*/) noexcept {
    return io::write_file(file, iovs);
}

result<size_t> stream_console_device::control(file &file, control_code_t code, std::span<const std::byte> in_buffer,
                                              std::span<std::byte> out_buffer) noexcept {
    return io::control_file(file, code, in_buffer, out_buffer);
}

result<void> stream_console_driver::install_device(stream_console_device &device,
                                                   chino::os::device &stream_device) noexcept {
    try_(device.install(stream_device));
    try_(io::attach_device(device));
    return ok();
}
