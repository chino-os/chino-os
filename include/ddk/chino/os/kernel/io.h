// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "kernel_types.h"
#include "ob.h"
#include <chino/intrusive_list.h>
#include <chino/os/file.h>
#include <chino/os/hal/arch.h>
#include <chino/os/kernel/ps.h>
#include <span>

namespace chino::os {
using control_code_t = uint32_t;

namespace kernel::io {
inline constexpr size_t max_io_parameters_size = sizeof(uintptr_t) * 16;

enum class io_frame_major_kind : uint16_t {
    generic,
};

struct io_frame_kind {
    io_frame_major_kind major;
    uint16_t minor;
};

enum class io_frame_generic_kind : uint16_t {
    open,
    read,
    write,
    control,
};

struct io_status {
    error_code code;
    union {
        size_t bytes_transferred;
    };
};

struct io_frame_params_generic {
    union {
        struct {
            std::string_view path;
            create_disposition create_disposition;
        } open;

        struct {
            std::span<std::byte> buffer;
            std::optional<size_t> offset;
        } read;

        struct {
            std::span<const std::byte> buffer;
            std::optional<size_t> offset;
        } write;

        struct {
            control_code_t code;
            void *arg;
        } control;
    };
};

class io_frame {
  public:
    constexpr io_frame(io_frame_kind kind, file &file) noexcept : kind_(kind), file_(file) {}

    io_frame_kind kind() const noexcept { return kind_; }
    os::file &file() const noexcept { return file_; }

    template <class T> T &params() noexcept {
        static_assert(sizeof(T) <= max_io_parameters_size && std::is_trivially_destructible_v<T>);
        return *reinterpret_cast<T *>(params_.data());
    }

  public:
    io_frame *next;

  private:
    io_frame_kind kind_;
    os::file &file_;
    alignas(std::max_align_t) std::array<std::byte, max_io_parameters_size> params_;
};

class io_request {
  public:
    io_frame &current_frame() const noexcept { return *current_frame_; }

    void complete(result<void> r) noexcept;

    void complete(result<size_t> r) noexcept {
        if (r.is_ok()) {
            status.bytes_transferred = r.unwrap();
            complete(ok());
        } else {
            complete(result<void>(r.unwrap_err()));
        }
    }

  public:
    intrusive_list_node request_list_node;

  private:
    io_status status;
    io_frame *current_frame_ = frames;
    io_frame frames[3];
};

struct io_request_open : io_request {
    std::string_view path;
    create_disposition create_disposition;
};

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
    void process_queued_ios() noexcept;

    virtual result<void> process_io(io_request &irp) noexcept;
    virtual result<void> cancel_io(io_request &irp) noexcept;
    virtual result<void> on_io_completion(io_request &irp) noexcept;

  public:
    intrusive_list_node work_list_node;

  private:
    ps::irq_spin_lock irps_lock_;
    io_request *current_irp_ = nullptr;
    intrusive_list<io_request, &io_request::request_list_node> irps_;
};

typedef result<void> (*irq_handler_t)(hal::arch_irq_number_t irq_number, void *context);

result<void> register_irq_handler(hal::arch_irq_number_t irq_number, irq_handler_t handler, void *context) noexcept;
void register_device_process_io(device &device) noexcept;

result<void> attach_device(device &device) noexcept;

result<file> open_file(device &device, std::string_view path, create_disposition disposition) noexcept;
result<file> open_file(std::string_view path, create_disposition disposition) noexcept;

result<void> close_file(file &file) noexcept;

result<size_t> read_file(file &file, std::span<std::byte> buffer, std::optional<size_t> offset = std::nullopt) noexcept;

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
