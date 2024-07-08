// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once

namespace chino::os {
enum class thread_priority { idle = 0, lowest = 1, low = 2, normal = 3, high = 4, highest = 5, max = highest };
enum class thread_status { ready = 0, running = 1, blocked = 2, delayed = 3, terminated = 4, max = terminated };

typedef int (*thread_start_t)(void *);
} // namespace chino::os
