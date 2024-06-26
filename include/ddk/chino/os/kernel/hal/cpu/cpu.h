// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <chino/compiler.h>

#ifdef CHINO_CPU_EMULATOR
#include "emulator/cpu.h"
#endif

namespace chino::os::kernel::hal {
inline constexpr size_t cacheline_size = 64;
}
