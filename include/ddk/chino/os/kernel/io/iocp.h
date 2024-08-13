// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <chino/intrusive_list.h>
#include <chino/os/ioapi.h>

namespace chino::os::kernel::ps {
class process;
}

namespace chino::os::kernel::io {
struct io_status {
    error_code code;
    union {
        size_t bytes_transferred;
    };
};

class async_io_block {
  public:
    static chino::result<async_io_block *> allocate(async_io_result &result) noexcept;

    constexpr async_io_block(ps::process &process, async_io_result &result) noexcept
        : process(process), result(result) {}

    void queue() noexcept;

  public:
    intrusive_list_node list_node;
    ps::process &process;
    async_io_result &result;
};
} // namespace chino::os::kernel::io
