// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "../compiler.h"
#include "../result.h"
#include "objectapi.h"
#include <span>
#include <string_view>

namespace chino::os {
using control_code_t = uint32_t;

struct async_io_result {
    error_code error;
    size_t bytes_transferred;
};

result<async_io_result *> wait_queued_io() noexcept;
result<void> read_async(int fd, std::span<std::byte> buffer, size_t offset, async_io_result &result) noexcept;
} // namespace chino::os
