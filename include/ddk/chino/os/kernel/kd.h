// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <chino/compiler.h>

namespace chino::os::kernel {
[[noreturn]] void ke_bugcheck(const char *format, ...) noexcept;
} // namespace chino::os::kernel

#ifndef NDEBUG
#define kassert(x)                                                                                                     \
    if (!(x)) {                                                                                                        \
        chino::os::kernel::ke_bugcheck("Assertion at %s:%d: %s", __FILE__, __LINE__, #x);                              \
    }
#else
#define kassert(x)
#endif
