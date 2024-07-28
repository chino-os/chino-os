// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "object.h"
#include <chino/os/objectapi.h>

namespace chino::os {
class handle_entry {
  public:
    CHINO_NONCOPYABLE(handle_entry);

    handle_entry(object &object, access_mask granted_access) noexcept
        : object_(&object), granted_access_(granted_access) {}

    os::object &object() const noexcept { return *object_; }
    access_mask granted_access() const noexcept { return granted_access_; }

  private:
    object_ptr<os::object> object_;
    access_mask granted_access_;
};
} // namespace chino::os
