// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "object.h"
#include <chino/os/kernel/kd.h>
#include <chino/os/kernel/ps.h>
#include <chino/os/objectapi.h>

namespace chino::os {
class file {
  public:
    CHINO_NONCOPYABLE(file);

    constexpr file() noexcept {}
    ~file();

    object &object() const noexcept { return *object_; }
    access_mask granted_access() const noexcept { return granted_access_; }

    template <class T> T *data() const noexcept { return reinterpret_cast<T *>(data_); }
    void data(void *value) noexcept { data_ = value; }

    kernel::ps::event &event() noexcept { return event_; }

    void prepare_to_open(os::object &object, access_mask desired_access) noexcept {
        kassert(object_.empty());
        object_ = &object;
        granted_access_ = desired_access;
    }

    void failed_to_open() noexcept {
        object_ = nullptr;
        granted_access_ = access_mask::none;
    }

  private:
    object_ptr<os::object> object_;
    access_mask granted_access_ = access_mask::none;
    void *data_ = nullptr;
    kernel::ps::event event_;
};
} // namespace chino::os
