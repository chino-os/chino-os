// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <cstddef>
#include <cstdint>

namespace chino {
enum class error_code : int32_t {
    success = 0,
    argument_null = 1,
    invalid_argument = 2,
    out_of_memory = 3,
    not_found = 4,
    unavailable = 5,
    key_already_exists = 6,
    not_implemented = 7,
    not_supported = 8,
    invalid_path = 9,
    insufficient_buffer = 10,
    io_error = 11,
    bad_cast = 12,
    timeout = 13,
};
}
