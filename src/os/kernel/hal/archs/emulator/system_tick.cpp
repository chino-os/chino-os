// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "emulator_cpu.h"
#include <chino/os/kernel/ke.h>

using namespace chino;
using namespace chino::os::kernel;
using namespace chino::os::kernel::hal;

namespace {}

void emulator_cpu::enable_system_tick(std::chrono::milliseconds ticks) {
    enable_systick_call call;
    call.ticks_in_ms = ticks.count();
    send_arch_call(call);
}

void emulator_cpu::on_system_tick() {
    wchar_t chars[] = L"Hello \n";
    WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), chars, std::size(chars), nullptr, nullptr);
    send_irq(arch_irq_number_t::system_tick);
}

void emulator_cpu::do_enable_systick(enable_systick_call *call) {
    SetTimer(event_window_, systick_timer_id, call->ticks_in_ms, nullptr);
}
