// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "process.h"
#include "sched/scheduler.h"

using namespace chino::os::kernel;
using namespace chino::os::kernel::ps;

void process::attach_thread(thread &thread) noexcept { threads_.push_back(&thread); }
