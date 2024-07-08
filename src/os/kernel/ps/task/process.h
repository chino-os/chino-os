// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "thread.h"

namespace chino::os::kernel::ps {
class process : public object {
    using process_list_t = intrusive_list<thread, &thread::process_list_node>;

    CHINO_DEFINE_KERNEL_OBJECT_KIND(object, object_kind_process);

  public:
    constexpr process() noexcept {};

    void attach_thread(thread &thread) noexcept;

  private:
    process_list_t threads_;
};
} // namespace chino::os::kernel::ps
