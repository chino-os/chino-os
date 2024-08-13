// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once

#if defined(CHINO_ARCH_RISCV32)
#include "riscv32/arch.h"
#else
#error "Unknown arch."
#endif
