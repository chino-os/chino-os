// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "../object.h"
#include <span>
#include <sys/uio.h>

namespace chino::os {
class device;

enum class std_handles { in, out, err };

enum class create_disposition {
    create_always = 2,
    create_new = 1,
    open_always = 4,
    open_existing = 3,
    truncate_existing = 5
};

using control_code_t = uint32_t;

class file : public object {
    CHINO_DEFINE_KERNEL_OBJECT_KIND(object, object_kind_file);

  public:
    file(device &device) noexcept : device_(&device) {}

    uint64_t offset() const noexcept { return offset_; }
    void offset(uint64_t value) noexcept { offset_ = value; }

    device &device() const noexcept { return *device_; }

  private:
    uint64_t offset_ = 0;
    object_ptr<os::device> device_;
};

class device : public object {
    CHINO_DEFINE_KERNEL_OBJECT_KIND(object, object_kind_device);

  public:
    virtual result<object_ptr<file>> open(std::string_view path, create_disposition create_disposition) noexcept = 0;
    virtual result<void> close(file &file) noexcept = 0;

    result<size_t> read(file &file, std::span<const iovec> iovs) noexcept;
    result<size_t> write(file &file, std::span<iovec> iovs) noexcept;

    virtual result<size_t> read(file &file, std::span<const iovec> iovs, size_t offset) noexcept = 0;
    virtual result<size_t> write(file &file, std::span<const iovec> iovs, size_t offset) noexcept = 0;
    virtual result<size_t> control(file &file, control_code_t code, std::span<const std::byte> in_buffer,
                                   std::span<std::byte> out_buffer) noexcept = 0;
};

namespace kernel::io {
result<void> attach_device(device &device) noexcept;

result<object_ptr<file>> open_file(device &device, std::string_view path,
                                   create_disposition create_disposition) noexcept;
result<void> close_file(file &file) noexcept;

result<size_t> read_file(file &file, std::span<const iovec> iovs, size_t offset) noexcept;
result<size_t> read_file(file &file, std::span<const iovec> iovs) noexcept;
result<size_t> read_file(file &file, std::span<std::byte> buffer, size_t offset) noexcept;
result<size_t> read_file(file &file, std::span<std::byte> buffer) noexcept;

result<size_t> write_file(file &file, std::span<const iovec> iovs, size_t offset) noexcept;
result<size_t> write_file(file &file, std::span<const iovec> iovs) noexcept;
result<size_t> write_file(file &file, std::span<const std::byte> buffer, size_t offset) noexcept;
result<size_t> write_file(file &file, std::span<const std::byte> buffer) noexcept;

result<size_t> control_file(file &file, control_code_t code, std::span<const std::byte> in_buffer,
                            std::span<std::byte> out_buffer) noexcept;

result<void> allocate_console() noexcept;
} // namespace kernel::io
} // namespace chino::os
