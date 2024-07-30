// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "../ps/task/process.h"
#include <chino/os/kernel/io.h>
#include <chino/os/kernel/io/device.h>
#include <chino/os/kernel/io/file.h>
#include <chino/os/kernel/kd.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::kernel::io;
using namespace chino::os::kernel::ps;

constinit object_pool<async_io_block> io_block_pool_;

result<async_io_block *> async_io_block::allocate(async_io_result &result) noexcept {
    try_var(r, io_block_pool_.allocate(ps::current_process(), result));
    return ok(r.first);
}

void async_io_block::queue() noexcept { process.queue_completed_io(*this); }

result<async_io_result *> io::io_wait_queued_io() noexcept {
    auto &block = current_process().dequeue_completed_io();
    auto &result = block.result;
    (void)io_block_pool_.free(&block);
    return ok(&result);
}

void process::queue_completed_io(io::async_io_block &block) noexcept {
    std::unique_lock<decltype(ready_io_blocks_lock_)> locker(ready_io_blocks_lock_);
    ready_io_blocks_.push_back(&block);
    if (!ready_io_blocks_.empty())
        ready_io_blocks_avail_.notify_all();
}

io::async_io_block &process::dequeue_completed_io() noexcept {
    while (true) {
        ready_io_blocks_avail_.wait().expect("Wait IOCP failed");
        std::unique_lock<decltype(ready_io_blocks_lock_)> locker(ready_io_blocks_lock_);
        auto head = ready_io_blocks_.front();
        if (head) {
            ready_io_blocks_.remove(head);
            return *head;
        }
    }
}
