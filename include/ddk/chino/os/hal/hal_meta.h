// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <chino/compiler.h>
#include <chino/uuid.h>
#include <string_view>
#include <vector>

namespace chino::os::hal::meta {
enum class device_meta_kind {
    simple,
    bus,
};

struct reg_range {
    uintptr_t start;
    uintptr_t size;
};
} // namespace chino::os::hal::meta
