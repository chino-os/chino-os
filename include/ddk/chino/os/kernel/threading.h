// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "object.h"

namespace chino::os::kernel::ps {
enum class thread_priority { idle = 0, lowest = 1, low = 2, normal = 3, high = 4, highest = 5, max = highest };

typedef int (*thread_start_t)(void *);

struct current_schedule_lock {
    CHINO_NONCOPYABLE(current_schedule_lock);

    current_schedule_lock() noexcept;
    ~current_schedule_lock();
};

class current_irq_schedule_lock {
  public:
    CHINO_NONCOPYABLE(current_irq_schedule_lock);

    current_irq_schedule_lock() noexcept;
    ~current_irq_schedule_lock();

  private:
};

class wait_queue {
  public:
  private:
};
} // namespace chino::os::kernel::ps
