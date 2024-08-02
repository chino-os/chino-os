// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <chino/os/kernel/ps.h>
#include <chino/os/object.h>
#include <chino/os/objectapi.h>

namespace chino::os::kernel::io {
class device;

inline constexpr size_t max_file_data_size = sizeof(uintptr_t) * 4;

class file : public object {
    CHINO_DEFINE_KERNEL_OBJECT_KIND(object, object_kind_file);

  public:
    constexpr file() noexcept {}
    ~file();

    io::device &device() const noexcept { return *device_; }

    template <class T> const T &data() const noexcept { return *reinterpret_cast<const T *>(data_); }
    template <class T> T &data() noexcept { return *reinterpret_cast<T *>(data_); }

    template <class T, class... TArgs> void construct_data(TArgs &&...args) noexcept {
        static_assert(sizeof(T) <= max_file_data_size);
        std::construct_at(reinterpret_cast<T *>(data_), std::forward<TArgs>(args)...);
    }

    os::event &event() noexcept { return event_; }

    void prepare_to_open(io::device &device) noexcept;
    void failed_to_open() noexcept;

  private:
    io::device *device_ = nullptr;
    os::event event_;
    alignas(std::max_align_t) std::byte data_[max_file_data_size] = {};
};
} // namespace chino::os::kernel::io
