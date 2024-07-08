// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <atomic>
#include <chino/os/kernel/hal/arch.h>
#include <chino/os/kernel/ke.h>
#include <chino/result.h>
#include <numeric>

namespace chino::os::kernel::mm {

/** @brief The page frame allocator.
  * @see The basic idea is from * LLFree: Scalable and Optionally-Persistent Page-Frame Allocation * @2023 USENIX ATC
  *      https://www.usenix.org/conference/atc23/presentation/wrenger
  */
class physical_allocator {
  public:
    static void initialize_phase0(const boot_options &options) noexcept;

    static result<uintptr_t> allocate_page() noexcept;
};
} // namespace chino::os::kernel::mm
