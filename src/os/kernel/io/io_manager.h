// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <chino/os/kernel/io.h>
#include <chino/os/kernel/ke.h>

namespace chino::os::kernel::io {
result<void> initialize_phase1(const boot_options &options) noexcept;

[[noreturn]] int io_worker_main(void *) noexcept;
void process_queued_ios(io_request *wait_irp = nullptr);
} // namespace chino::os::kernel::io
