// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "io_manager.h"
#include "../ob/directory.h"
#include <chino/conf/board_init.inl>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::kernel::io;
using namespace std::string_view_literals;

#define BUFFER_TO_IOVS(buffer, iovs)                                                                                   \
    iovec iovs##0 {.iov_base = (void *)buffer.data(), .iov_len = buffer.size_bytes()};                                 \
    std::span<const iovec> iovs(&iovs##0, 1)

namespace {
class dev_directory : public ob::directory {
  public:
    std::string_view name() const noexcept override { return "dev"sv; }
};

constinit dev_directory dev_directory_;
} // namespace

result<void> io::initialize_phase1(const boot_options &options) noexcept {
    try_(hal::hal_install_devices());
    return ok();
}

result<void> io::attach_device(device &device) noexcept {
    try_(dev_directory_.insert(device, device.name()));
    return ok();
}

result<object_ptr<file>> io::open_file(device &device, std::string_view path,
                                       create_disposition create_disposition) noexcept {
    return device.open(path, create_disposition);
}

result<void> io::close_file(file &file) noexcept { return file.device().close(file); }

result<size_t> io::read_file(file &file, std::span<const iovec> iovs, size_t offset) noexcept {
    return file.device().read(file, iovs, offset);
}

result<size_t> io::read_file(file &file, std::span<const iovec> iovs) noexcept {
    auto offset = file.offset();
    try_var(read, read_file(file, iovs, offset));
    file.offset(offset + read);
    return ok(read);
}

result<size_t> io::read_file(file &file, std::span<std::byte> buffer, size_t offset) noexcept {
    BUFFER_TO_IOVS(buffer, iovs);
    return read_file(file, iovs, offset);
}

result<size_t> io::read_file(file &file, std::span<std::byte> buffer) noexcept {
    BUFFER_TO_IOVS(buffer, iovs);
    return read_file(file, iovs);
}

result<size_t> io::write_file(file &file, std::span<const iovec> iovs, size_t offset) noexcept {
    return file.device().write(file, iovs, offset);
}

result<size_t> io::write_file(file &file, std::span<const iovec> iovs) noexcept {
    auto offset = file.offset();
    try_var(written, write_file(file, iovs, offset));
    file.offset(offset + written);
    return ok(written);
}
result<size_t> io::write_file(file &file, std::span<const std::byte> buffer, size_t offset) noexcept {
    BUFFER_TO_IOVS(buffer, iovs);
    return write_file(file, iovs, offset);
}

result<size_t> io::write_file(file &file, std::span<const std::byte> buffer) noexcept {
    BUFFER_TO_IOVS(buffer, iovs);
    return write_file(file, iovs);
}

result<size_t> io::control_file(file &file, control_code_t code, std::span<const std::byte> in_buffer,
                                std::span<std::byte> out_buffer) noexcept {
    return file.device().control(file, code, in_buffer, out_buffer);
}

result<void> io::allocate_console() noexcept { return ok(); }
