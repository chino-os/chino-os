// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "stream_console.h"
#include <chino/os/kernel/io.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::drivers;

#define GET_BOTTOM_DEVICE(file) auto &dev = static_cast<device &>(file.object())

result<void> stream_console_device::install(device &stream_device) noexcept {
    try_set(stream_file_, io::open_file(stream_device, {}, create_disposition::open_existing));
    return ok();
}

result<size_t> stream_console_device::fast_read(file &file, std::span<std::byte> buffer,
                                                std::optional<size_t> /*offset*/) noexcept {
    GET_BOTTOM_DEVICE(stream_file_);
    return dev.fast_read(stream_file_, buffer);
}

result<size_t> stream_console_device::fast_write(file &file, std::span<const std::byte> buffer,
                                                 std::optional<size_t> /*offset*/) noexcept {
    GET_BOTTOM_DEVICE(stream_file_);
    return dev.fast_write(stream_file_, buffer);
}

result<void> stream_console_driver::install_device(stream_console_device &device, io::device &stream_device) noexcept {
    try_(device.install(stream_device));
    try_(io::attach_device(device));
    return ok();
}
