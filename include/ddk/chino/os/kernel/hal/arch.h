// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <chino/compiler.h>

#ifdef CHINO_ARCH_EMULATOR
#include "archs/emulator/arch.h"
#else
#error "Unknown arch."
#endif

namespace chino::os::kernel::hal {
inline constexpr size_t cacheline_size = 64;
}
