// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include <chino/os/hal/chip.h>
#include <chino/os/kernel/kd.h>
#include <chino/result.h>
#ifdef WIN32
#include <Windows.h>
#endif

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;

static char bugcheck_buffer_[128];

void kernel::ke_bugcheck(const char *format, ...) noexcept {
    va_list ap;
    va_start(ap, format);
    vsnprintf(bugcheck_buffer_, std::size(bugcheck_buffer_), format, ap);
    hal::chip_t::debug_print(bugcheck_buffer_);
#ifdef WIN32
    DebugBreak();
#endif // WIN32
    while (1)
        ;
}

void chino::fail_fast(const char *message) noexcept { ke_bugcheck("FAIL: %s\n", message); }
