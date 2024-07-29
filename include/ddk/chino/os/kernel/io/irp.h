// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "file.h"
#include <chino/intrusive_list.h>
#include <chino/os/ioapi.h>
#include <chino/os/kernel/object_pool.h>
#include <span>

namespace chino::os::kernel::io {
class file;

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

struct io_status {
    error_code code;
    union {
        size_t bytes_transferred;
    };
};

template <io_frame_major_kind Major> struct io_frame_minor_kind_traits;

enum class io_frame_generic_kind : uint16_t {
    open,
    read,
    write,
    control,
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

    template <io_frame_generic_kind Minor> auto &by_minor() noexcept {
        if constexpr (Minor == io_frame_generic_kind::open) {
            return open;
        } else if constexpr (Minor == io_frame_generic_kind::read) {
            return read;
        } else if constexpr (Minor == io_frame_generic_kind::write) {
            return write;
        } else if constexpr (Minor == io_frame_generic_kind::control) {
            return control;
        } else {
            return;
        }
    }
};

template <> struct io_frame_minor_kind_traits<io_frame_major_kind::generic> {
    using minor_type = io_frame_generic_kind;
    using params_type = io_frame_params_generic;
};

class io_frame {
  public:
    io_frame(io_frame_kind kind, file &file) noexcept : kind_(kind), file_(&file) {}

    io_frame_kind kind() const noexcept { return kind_; }
    io::file &file() const noexcept { return *file_; }

    template <class T> T &params() noexcept {
        static_assert(sizeof(T) <= max_io_parameters_size && std::is_trivially_destructible_v<T>);
        return *reinterpret_cast<T *>(params_.data());
    }

    template <io_frame_major_kind Major, io_frame_minor_kind_traits<Major>::minor_type Minor> auto &params() noexcept {
        using params_type = typename io_frame_minor_kind_traits<Major>::params_type;
        return params<params_type>().template by_minor<Minor>();
    }

  private:
    io_frame_kind kind_;
    object_ptr<io::file> file_;
    alignas(std::max_align_t) std::array<std::byte, max_io_parameters_size> params_;
};

class io_request : public object {
    CHINO_DEFINE_KERNEL_OBJECT_KIND(object, object_kind_irp);

  public:
    static result<object_ptr<io_request>> allocate(io_frame_kind kind, file &file) noexcept;

    io_request(io_frame_kind kind, file &file) noexcept
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

    void complete(error_code error) noexcept { complete(result<void>(error)); }

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
} // namespace chino::os::kernel::io
