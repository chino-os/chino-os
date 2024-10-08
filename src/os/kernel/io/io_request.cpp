// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "io_manager.h"
#include <chino/os/kernel/io.h>
#include <chino/os/kernel/kd.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::kernel::io;
using namespace chino::os::kernel::ps;

constinit object_pool<io_request> io_request_pool_;

template <> void object_pool<io_request>::object_pool_object::internal_release() noexcept {
    (void)io_request_pool_.free(this);
}

result<object_ptr<io_request>> io_request::allocate(async_io_block *async_io_block, io_frame_kind kind,
                                                    file &file) noexcept {
    try_var(r, io_request_pool_.allocate(async_io_block, kind, file));
    return ok(r.first);
}

io_request::~io_request() {}

result<io_frame *> io_request::move_next_frame(io_frame_kind kind, file &file) noexcept {
    if (current_frame_ != frames_ + std::size(frames_) - 1) {
        current_frame_++;
        std::construct_at(current_frame_, kind, file);
        return ok(current_frame_);
    }
    return err(error_code::out_of_memory);
}

void io_request::queue() noexcept {
    if (current_frame_ == frames_)
        add_ref();
    current_frame_->file().device().queue_io(*this);
}

void io_request::complete() noexcept {
    kassert(!is_completed());
    kassert(!hal::arch_t::in_irq_handler());
    kassert(status_.code != error_code::io_pending); // Must set status
    current_frame_->file().event().notify_all();
    std::destroy_at(current_frame_);
    if (current_frame_ == frames_) {
        // IRP completed
        current_frame_ = nullptr;
        if (async_io_block_) {
            async_io_block_->result = {.error = status_.code, .bytes_transferred = status_.bytes_transferred};
            async_io_block_->queue();
        }
    } else {
        current_frame_--;
        current_frame_->file().device().on_io_completion(*this);
    }
}

result<void> io_request::internal_wait() noexcept {
    if (!is_io_worker_thread(current_thread())) {
        return frames_[0].file().event().wait();
    } else {
        if (!is_completed())
            io::process_queued_ios(this);
        return ok();
    }
}
