// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <chino/os/kernel/ke.h>

namespace chino::os::kernel::mm {
void initialize_phase0(const boot_options &options) noexcept;
}
