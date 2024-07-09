// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "emulator_cpu.h"
#include <chino/os/kernel/ke.h>

using namespace chino;
using namespace chino::os::kernel;
using namespace chino::os::kernel::hal;

namespace {}

void emulator_cpu::on_system_tick_timer(_Inout_ PTP_CALLBACK_INSTANCE Instance, _Inout_opt_ PVOID Context,
                                        _Inout_ PTP_TIMER Timer) {
    auto cpu = (emulator_cpu *)Context;
    cpu->send_irq(arch_irq_number_t::system_tick);
}

void emulator_cpu::enable_system_tick(std::chrono::milliseconds ticks) {
    int64_t dueTime = -(int64_t)ticks.count() * 10000; // in 100ns
    SetThreadpoolTimer(systick_timer_, (FILETIME *)&dueTime, 0, 0);
}
