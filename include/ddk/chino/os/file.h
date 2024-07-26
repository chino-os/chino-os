// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "object.h"
#include <chino/os/objectapi.h>

namespace chino::os {
class file {
  public:
    CHINO_NONCOPYABLE(file);

    constexpr file() noexcept : object_(nullptr), granted_access_(access_mask::none), data_(nullptr) {}
    file(os::object &object, access_mask granted_access, void *data = nullptr) noexcept
        : object_(&object), granted_access_(granted_access), data_(data) {}
    file(file &&) noexcept = default;
    ~file();

    file &operator=(file &&) noexcept = default;

    object &object() const noexcept { return *object_; }
    access_mask granted_access() const noexcept { return granted_access_; }

    template <class T> T *data() const noexcept { return reinterpret_cast<T *>(data_); }
    void data(void *value) noexcept { data_ = value; }

  private:
    object_ptr<os::object> object_;
    access_mask granted_access_;
    void *data_;
};
} // namespace chino::os
