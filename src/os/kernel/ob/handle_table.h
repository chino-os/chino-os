// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <array>
#include <chino/os/handle.h>
#include <chino/os/kernel/object_pool.h>
#include <unistd.h>

namespace chino::os::kernel::ob {
inline static int fd_base = STDERR_FILENO + 1;

class handle_table : private object_pool<handle_entry> {
  public:
    result<std::pair<handle_entry *, int>> allocate(object &object, access_mask granted_access) noexcept {
        try_var(r, object_pool::allocate(object, granted_access));
        return ok(std::make_pair(r.first, (int)r.second + fd_base));
    }

    result<void> free(int fd) noexcept {
        if (fd < fd_base)
            return err(error_code::invalid_argument);
        return object_pool::free(fd - fd_base);
    }

    result<handle_entry *> at(int fd) noexcept {
        if (fd < fd_base)
            return err(error_code::invalid_argument);
        return object_pool::at(fd - fd_base);
    }
};
} // namespace chino::os::kernel::ob
