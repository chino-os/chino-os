// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "device.h"
#include <chino/os/kernel/kd.h>
#include <chino/os/kernel/ps.h>
#include <chino/os/object.h>
#include <chino/os/objectapi.h>

namespace chino::os::kernel::io {
class file : public object {
    CHINO_DEFINE_KERNEL_OBJECT_KIND(object, object_kind_file);

  public:
    constexpr file() noexcept {}
    ~file();

    io::device &device() const noexcept { return *device_; }

    template <class T> T *data() const noexcept { return reinterpret_cast<T *>(data_); }
    void data(void *value) noexcept { data_ = value; }

    os::event &event() noexcept { return event_; }

    void prepare_to_open(io::device &device) noexcept {
        kassert(device_.empty());
        device_ = &device;
    }

    void failed_to_open() noexcept { device_ = nullptr; }

  private:
    object_ptr<io::device> device_;
    void *data_ = nullptr;
    os::event event_;
};
} // namespace chino::os::kernel::io
