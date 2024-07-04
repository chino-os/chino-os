// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "chino/os/kernel/object_kind.h"
#include <atomic>
#include <chino/instrusive_list.h>
#include <chino/os/kernel/hal/arch.h>
#include <chino/os/kernel/object.h>
#include <chino/os/kernel/threading.h>

namespace chino::os::kernel::ps {
struct thread_context : public hal::arch_thread_context_t {};

class thread : public object {
    CHINO_DEFINE_KERNEL_OBJECT_KIND(object, object_kind_thread);

  public:
    constexpr thread() noexcept : context{}, priority_(thread_priority::idle){};

    thread_priority priority() const noexcept { return priority_; }

  public:
    intrusive_list_node schedule_list_node;
    thread_context context;

  private:
    thread_priority priority_;
};
} // namespace chino::os::kernel::ps
