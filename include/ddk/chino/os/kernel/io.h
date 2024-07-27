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
inline constexpr size_t max_io_parameters_size = sizeof(uintptr_t) * 8;

enum class io_frame_major_kind : uint16_t {
    generic,
};

struct io_frame_kind {
    io_frame_major_kind major;
    uint16_t minor;
};

template <class TMinor> constexpr io_frame_kind make_io_frame_kind(io_frame_major_kind major, TMinor minor) noexcept {
    return {.major = major, .minor = static_cast<uint16_t>(minor)};
}

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

  private:
    io_frame_kind kind_;
    os::file &file_;
    alignas(std::max_align_t) std::array<std::byte, max_io_parameters_size> params_;
};

class io_request {
  public:
    constexpr io_request(io_frame_kind kind, file &file) noexcept
        : status_{.code = error_code::io_pending, .bytes_transferred = 0}, current_frame_(frames_) {
        std::construct_at(current_frame_, kind, file);
    }

    ~io_request();

    result<io_frame *> current_frame() const noexcept {
        return current_frame_ ? ok(current_frame_) : err(error_code::unavailable);
    }

    result<io_frame *> move_next_frame(io_frame_kind kind, file &file) noexcept;

    bool is_completed() const noexcept { return !current_frame_; }
    io_status status() const noexcept { return status_; }

    result<void> queue() noexcept;

    void complete() noexcept;

    template <class T> void complete(result<T> r) noexcept {
        status_.code = r.unwrap_err();
        if (r.is_ok()) {
            if constexpr (!std::is_void_v<T>) {
                status_.bytes_transferred = static_cast<T>(r.unwrap());
            }
        }
        complete();
    }

    template <class T = void> result<T> wait() noexcept {
        try_(internal_wait());
        if (status_.code == error_code::success) {
            if constexpr (std::is_void_v<T>)
                return ok();
            else
                return ok(static_cast<T>(status_.bytes_transferred));
        } else {
            return err(status_.code);
        }
    }

  private:
    result<void> internal_wait() noexcept;

  public:
    intrusive_list_node request_list_node;

  private:
    io_status status_;
    union {
        io_frame frames_[3];
    };
    io_frame *current_frame_;
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

typedef result<void> (*irq_handler_t)(hal::arch_irq_number_t irq_number, void *context);

result<void> register_irq_handler(hal::arch_irq_number_t irq_number, irq_handler_t handler, void *context) noexcept;
void register_device_process_io(device &device) noexcept;

result<void> attach_device(device &device) noexcept;

result<void> open_file(file &file, access_mask desired_access, device &device, std::string_view path,
                       create_disposition disposition) noexcept;
result<void> open_file(file &file, access_mask desired_access, std::string_view path,
                       create_disposition disposition) noexcept;

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
