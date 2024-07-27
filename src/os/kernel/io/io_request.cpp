// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "io_manager.h"
#include <chino/os/kernel/io.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::kernel::io;
using namespace chino::os::kernel::ps;

io_request::~io_request() {}

result<io_frame *> io_request::move_next_frame(io_frame_kind kind, file &file) noexcept {
    if (current_frame_ != frames_ + std::size(frames_) - 1) {
        current_frame_++;
        std::construct_at(current_frame_, kind, file);
        return ok(current_frame_);
    }
    return err(error_code::out_of_memory);
}

result<void> io_request::queue() noexcept {
    auto &dev = static_cast<device &>(current_frame_->file().object());
    return dev.queue_io(*this);
}

void io_request::complete() noexcept {
    kassert(!is_completed());
    kassert(status_.code != error_code::io_pending); // Must set status
    current_frame_->file().event().notify_all();
    std::destroy_at(current_frame_);
    if (current_frame_ == frames_) {
        // IRP completed
        current_frame_ = nullptr;
    } else {
        current_frame_--;
        auto &dev = static_cast<device &>(current_frame_->file().object());
        dev.on_io_completion(*this);
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
