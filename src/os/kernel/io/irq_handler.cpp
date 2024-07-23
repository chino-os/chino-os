// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "../ps/sched/scheduler.h"
#include <chino/os/kernel/io.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::kernel::io;
using namespace chino::os::hal;

struct irq_handler_entry {
    irq_handler_t handler;
    void *context;
};

static std::atomic<uint32_t> in_irq_handler_;
static std::array<irq_handler_entry, (size_t)arch_irq_number_t::__count> irq_handlers_;

struct in_irq_guard {
    in_irq_guard() noexcept { in_irq_handler_.store(1, std::memory_order_release); }
    ~in_irq_guard() { in_irq_handler_.store(0, std::memory_order_release); }
};

result<void> io::register_irq_handler(hal::arch_irq_number_t irq_number, irq_handler_t handler,
                                      void *context) noexcept {
    irq_handlers_[(size_t)irq_number] = {.handler = handler, .context = context};
    return ok();
}

bool io::in_irq_handler() noexcept { return in_irq_handler_.load(std::memory_order_acquire); }

static void io_handle_syscall(syscall_number number, void *arg) noexcept {
    switch (number) {
    case syscall_number::yield:
        ps::scheduler::current().switch_task();
        break;
    default:
        break;
    }
}

void io_handle_irq(arch_irq_number_t irq_number, syscall_number number, void *arg) noexcept {
    in_irq_guard irq_guard;
    switch (irq_number) {
    case arch_irq_number_t::system_tick:
        ps::scheduler::current().on_system_tick();
        break;
    case arch_irq_number_t::syscall:
        io_handle_syscall(number, arg);
        break;
    default:
        auto handler = irq_handlers_[(size_t)irq_number];
        if (!handler.handler)
            fail_fast("No handler found for IRQ.");
        handler.handler(irq_number, handler.context).expect("Failed to handle IRQ.");
        break;
    }
}
