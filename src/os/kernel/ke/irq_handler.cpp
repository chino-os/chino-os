// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "../ps/sched/scheduler.h"
#include <chino/os/kernel/ke.h>

using namespace chino;
using namespace chino::os::kernel;
using namespace chino::os::kernel::hal;

void ke_handle_irq(hal::arch_irq_number_t irq_number) noexcept {
    switch (irq_number) {
    case arch_irq_number_t::system_tick:
        ps::scheduler::current().on_system_tick();
        break;
    default:
        break;
    }
}
