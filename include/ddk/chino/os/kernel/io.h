// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "io/device.h"
#include "io/file.h"
#include "io/iocp.h"
#include "kernel_types.h"
#include "ob.h"
#include <chino/intrusive_list.h>
#include <chino/os/hal/arch.h>
#include <chino/os/ioapi.h>
#include <chino/os/objectapi.h>

namespace chino::os::kernel::io {
typedef result<void> (*irq_handler_t)(hal::arch_irq_number_t irq_number, void *context);

result<void> register_irq_handler(hal::arch_irq_number_t irq_number, irq_handler_t handler, void *context) noexcept;
void register_device_process_io(device &device) noexcept;

result<void> attach_device(device &device) noexcept;

result<void> open_file(file &file, access_mask desired_access, device &device, std::string_view path,
                       create_disposition disposition) noexcept;
result<void> open_file(file &file, access_mask desired_access, std::string_view path,
                       create_disposition disposition) noexcept;
result<object_ptr<file>> open_file(access_mask desired_access, std::string_view path,
                                   create_disposition disposition) noexcept;

result<void> close_file(file &file) noexcept;

result<size_t> read_file(file &file, std::span<std::byte> buffer, std::optional<size_t> offset = std::nullopt) noexcept;
result<void> read_file_async(file &file, std::span<std::byte> buffer, size_t offset, async_io_result &result) noexcept;

result<size_t> write_file(file &file, std::span<const std::byte> buffer,
                          std::optional<size_t> offset = std::nullopt) noexcept;

result<int> control_file(file &file, control_code_t code, void *arg) noexcept;

result<async_io_result *> io_wait_queued_io() noexcept;

result<void> allocate_console() noexcept;
} // namespace chino::os::kernel::io

extern "C" {
void io_handle_irq(chino::os::hal::arch_irq_number_t irq_number, chino::os::kernel::syscall_number number,
                   void *arg) noexcept;
}
