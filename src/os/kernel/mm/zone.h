// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "physical_page.h"

namespace chino::os::kernel::mm {
class free_area {
  private:
    std::atomic<size_t> segments_count_;
};

class zone {
  public:
    inline static constexpr size_t max_rank = 10;

    constexpr zone() : avail_pages_(0) {}

  private:
    std::atomic<size_t> avail_pages_;
    free_area free_areas_[max_rank + 1];
};
} // namespace chino::os::kernel::mm
