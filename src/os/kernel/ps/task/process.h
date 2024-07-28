// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "../../ob/handle_table.h"
#include "thread.h"
#include <chino/os/kernel/io/file.h>
#include <chino/os/kernel/object_pool.h>

namespace chino::os::kernel::ps {
class process : public object {
    using process_list_t = intrusive_list<thread, &thread::process_list_node>;

    CHINO_DEFINE_KERNEL_OBJECT_KIND(object, object_kind_process);

  public:
    constexpr process() noexcept {};

    ob::handle_table &handle_table() noexcept { return handle_table_; }
    object_pool<io::file> &file_table() noexcept { return file_table_; }

    void attach_thread(thread &thread) noexcept;

  private:
    process_list_t threads_;
    ob::handle_table handle_table_;
    object_pool<io::file> file_table_;
};
} // namespace chino::os::kernel::ps
