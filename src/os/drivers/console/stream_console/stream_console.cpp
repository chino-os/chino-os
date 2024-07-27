// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "stream_console.h"
#include <chino/os/kernel/io.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::kernel::io;
using namespace chino::os::drivers;

#define GET_BOTTOM_DEVICE(file) auto &dev = static_cast<device &>(file.object())

result<void> stream_console_device::install(device &stream_device) noexcept {
    try_(io::open_file(stream_file_, access_mask::generic_all, stream_device, {}, create_disposition::open_existing));
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

result<void> stream_console_device::process_io(kernel::io::io_request &irp) noexcept {
    try_var(cnt_frame, irp.current_frame());
    if (cnt_frame->kind().major == io_frame_major_kind::generic) {
        switch ((io_frame_generic_kind)cnt_frame->kind().minor) {
        case io_frame_generic_kind::read:
        case io_frame_generic_kind::write: {
            try_var(next_frame, irp.move_next_frame(cnt_frame->kind(), stream_file_));
            next_frame->params<io_frame_params_generic>() = cnt_frame->params<io_frame_params_generic>();
            try_(irp.queue());
            return ok();
        }
        default:
            break;
        }
    }
    return err(error_code::not_supported);
}

result<void> stream_console_driver::install_device(stream_console_device &device, io::device &stream_device) noexcept {
    try_(device.install(stream_device));
    try_(io::attach_device(device));
    return ok();
}
