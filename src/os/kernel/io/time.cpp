// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "../ke/ke_services.h"
#include "../ps/sched/scheduler.h"
#include "io_manager.h"

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::kernel::io;

int kernel_ke_service_mt::nanosleep(const struct timespec *rqtp, struct timespec *rmtp) noexcept {
    ps::scheduler::current().delay_current_thread(std::chrono::nanoseconds(rqtp->tv_sec * 1000000000 + rqtp->tv_nsec));
    if (rmtp)
        *rmtp = {};
    return 0;
}

int kernel_ke_service_mt::clock_gettime(clockid_t clock_id, struct timespec *tp) noexcept { return 0; }
