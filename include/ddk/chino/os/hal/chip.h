// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <chino/compiler.h>

#ifdef CHINO_ARCH_EMULATOR
#include "chips/emulator/chip.h"
#else
#error "Unknown chip."
#endif

namespace chino::os::hal {}
