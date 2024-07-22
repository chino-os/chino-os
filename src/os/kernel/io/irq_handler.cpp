// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "../ps/sched/scheduler.h"
#include <chino/os/kernel/io.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::hal;

struct irq_handler_entry {};

void io_handle_syscall(syscall_number number, void *arg) noexcept {
    switch (number) {
    case syscall_number::yield:
        ps::scheduler::current().switch_task();
        break;
    default:
        break;
    }
}

void io_handle_irq(arch_irq_number_t irq_number, syscall_number number, void *arg) noexcept {
    switch (irq_number) {
    case arch_irq_number_t::system_tick:
        ps::scheduler::current().on_system_tick();
        break;
    case arch_irq_number_t::syscall:
        io_handle_syscall(number, arg);
        break;
    default:
        break;
    }
}
