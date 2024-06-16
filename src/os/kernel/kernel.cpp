// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include <chino/os/kernel/kernel.h>

using namespace chino::os::kernel;

extern "C" [[noreturn]] void CHINO_KERNEL_ENTRY(const boot_context &context) {
    (void)context;
    while (1) {
    }
}
