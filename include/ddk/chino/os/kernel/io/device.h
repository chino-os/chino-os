// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "irp.h"
#include <chino/os/ioapi.h>
#include <chino/os/kernel/ob.h>
#include <chino/os/kernel/ps.h>

namespace chino::os::kernel::io {
class file;

class device : public kernel::ob::named_object {
    CHINO_DEFINE_KERNEL_OBJECT_KIND(named_object, object_kind_device);

  public:
    virtual result<void> close(file &file) noexcept;

    // Fast IO
    virtual result<void> fast_open(file &file, std::string_view path, create_disposition create_disposition) noexcept;
    virtual result<size_t> fast_read(file &file, std::span<std::byte> buffer,
                                     std::optional<size_t> offset = std::nullopt) noexcept;
    virtual result<size_t> fast_write(file &file, std::span<const std::byte> buffer,
                                      std::optional<size_t> offset = std::nullopt) noexcept;
    virtual result<size_t> fast_control(file &file, control_code_t code, void *arg) noexcept;

    // Slow IO
    io_request *current_irp() const noexcept { return current_irp_; }
    result<void> queue_io(io_request &irp) noexcept;
    bool process_queued_ios(io_request *wait_irp = nullptr) noexcept;

    virtual result<void> process_io(io_request &irp) noexcept;
    virtual result<void> cancel_io(io_request &irp) noexcept;
    virtual void on_io_completion(io_request &irp) noexcept;

  public:
    intrusive_list_node work_list_node;

  private:
    ps::irq_spin_lock irps_lock_;
    io_request *current_irp_ = nullptr;
    intrusive_list<io_request, &io_request::request_list_node> irps_;
};
} // namespace chino::os::kernel::io
