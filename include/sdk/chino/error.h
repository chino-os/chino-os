// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <cstddef>
#include <cstdint>

namespace chino {
enum class error_code : int32_t {
    success = 0,
    fail = 1,
    argument_null = 2,
    invalid_argument = 3,
    out_of_memory = 4,
    not_found = 5,
    unavailable = 6,
    key_already_exists = 7,
    not_implemented = 8,
    not_supported = 9,
    invalid_path = 10,
    insufficient_buffer = 11,
    io_error = 12,
    bad_cast = 13,
    timeout = 14,
    again = 15,
    slow_io = 16,
    io_pending = 17,
    message_too_long = 18,
};
}
