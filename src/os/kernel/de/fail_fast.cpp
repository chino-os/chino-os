// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include <chino/os/hal/chip.h>
#include <chino/result.h>

using namespace chino;

void chino::fail_fast(const char *message) noexcept {
    os::hal::chip_t::debug_print("FAIL: %s\n", message);
    while (1)
        ;
}
