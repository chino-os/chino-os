// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <atomic>
#include <chino/os/kernel/hal/cpu/cpu.h>
#include <chino/os/kernel/kernel.h>
#include <chino/result.h>
#include <numeric>

namespace chino::os::kernel::mm {
namespace detail {
struct physical_page_member_checker;
}

enum class physical_page_flags : uint32_t {
    chunk = 0b1,
};

class physical_page {
  public:
  private:
    friend struct detail::physical_page_member_checker;

    physical_page_flags flags_;
};

class physical_page_chunk {
  public:
  private:
    friend struct detail::physical_page_member_checker;

    union {
        physical_page_flags flags_;
        size_t page_index_;

        physical_page page_;
    };
};

namespace detail {
static_assert(sizeof(physical_page_chunk) <= hal::cacheline_size);

struct physical_page_member_checker {
    static_assert(offsetof(physical_page_chunk, flags_) == offsetof(physical_page, flags_));
};
} // namespace detail
} // namespace chino::os::kernel::mm
