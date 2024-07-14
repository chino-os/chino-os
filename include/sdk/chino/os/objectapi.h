// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "../compiler.h"

namespace chino::os {
enum class access_mask : uint32_t { none = 0, generic_all = 0b001, generic_read = 0b010, generic_write = 0b100 };
} // namespace chino::os
