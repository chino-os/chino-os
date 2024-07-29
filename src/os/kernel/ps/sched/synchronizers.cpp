// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "scheduler.h"
#include <chino/os/kernel/kd.h>

using namespace chino;
using namespace chino::os::kernel;
using namespace chino::os::kernel::ps;

current_schedule_lock::current_schedule_lock() noexcept { scheduler::current().lock(); }
current_schedule_lock::~current_schedule_lock() { scheduler::current().unlock(); }
