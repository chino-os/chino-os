// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "../object.h"
#include "kernel_types.h"
#include <chino/os/file.h>
#include <chino/os/hal/arch.h>
#include <span>
#include <sys/uio.h>

namespace chino::os {
using control_code_t = uint32_t;

class device : public object {
    CHINO_DEFINE_KERNEL_OBJECT_KIND(object, object_kind_device);

  public:
    virtual result<file> open(std::string_view path, create_disposition create_disposition) noexcept = 0;
    virtual result<file> duplicate(file &file) noexcept { return err(error_code::not_supported); };
    virtual result<void> close(file &file) noexcept = 0;

    virtual result<size_t> read(file &file, std::span<const iovec> iovs, std::optional<size_t> offset) noexcept = 0;
    virtual result<size_t> write(file &file, std::span<const iovec> iovs, std::optional<size_t> offset) noexcept = 0;
    virtual result<int> control(file &file, int request, void *arg) noexcept = 0;
};

namespace kernel::io {
typedef result<void> (*irq_handler_t)(hal::arch_irq_number_t irq_number, void *context);

result<void> register_irq_handler(hal::arch_irq_number_t irq_number, irq_handler_t handler, void *context) noexcept;

result<void> attach_device(device &device) noexcept;

result<file> open_file(device &device, std::string_view path, create_disposition disposition) noexcept;
result<file> open_file(std::string_view path, create_disposition disposition) noexcept;
result<void> close_file(file &file) noexcept;

result<size_t> read_file(file &file, std::span<const iovec> iovs, std::optional<size_t> offset = std::nullopt) noexcept;
result<size_t> read_file(file &file, std::span<std::byte> buffer, std::optional<size_t> offset = std::nullopt) noexcept;

result<size_t> write_file(file &file, std::span<const iovec> iovs,
                          std::optional<size_t> offset = std::nullopt) noexcept;
result<size_t> write_file(file &file, std::span<const std::byte> buffer,
                          std::optional<size_t> offset = std::nullopt) noexcept;

result<int> control_file(file &file, int request, void *arg) noexcept;

result<void> allocate_console() noexcept;
} // namespace kernel::io
} // namespace chino::os

extern "C" {
void io_handle_irq(chino::os::hal::arch_irq_number_t irq_number, chino::os::kernel::syscall_number number,
                   void *arg) noexcept;
}
