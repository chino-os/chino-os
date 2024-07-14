// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <array>
#include <chino/os/kernel/ps.h>
#include <chino/os/object.h>
#include <chino/os/objectapi.h>

namespace chino::os::kernel::ob {
struct handle_entry {
    object *object;
    access_mask granted_access;
};

namespace detail {
class handle_segment {
    struct free_entry {
        free_entry *next;
        bool used;
    };

    union element {
        free_entry free;
        handle_entry entry;
    };

  public:
    static constexpr size_t elements_count = (hal::arch_t::min_page_size - 3 * sizeof(uintptr_t)) / sizeof(element);

    constexpr handle_segment() noexcept : next(nullptr), frees_(elements_count), free_head_(&elements_->free) {
        for (size_t i = 0; i < elements_count - 1; i++) {
            elements_[i].free = {.next = &elements_[i + 1].free, .used = 0};
        }
    }

    result<std::pair<handle_entry *, size_t>> allocate() noexcept {
        std::unique_lock<decltype(lock_)> lock(lock_);
        if (free_head_) {
            auto item = reinterpret_cast<handle_entry *>(free_head_);
            free_head_ = free_head_->next;
            frees_.fetch_sub(1, std::memory_order_acq_rel);
            return ok(std::make_pair(item, index(item)));
        } else {
            return err(error_code::out_of_memory);
        }
    }

    void free(handle_entry *handle) noexcept {
        std::unique_lock<decltype(lock_)> lock(lock_);
        auto head = reinterpret_cast<free_entry *>(handle);
        head->next = free_head_;
        head->used = 0;
        free_head_ = head;
        frees_.fetch_add(1, std::memory_order_acq_rel);
    }

    result<handle_entry *> at(size_t index) noexcept {
        auto &element = elements_[index];
        if (!element.free.used)
            return err(error_code::invalid_argument);
        return ok(&element.entry);
    }

  private:
    size_t index(handle_entry *handle) const noexcept { return handle - &elements_->entry; }

  public:
    handle_segment *next;

  private:
    ps::irq_spin_lock lock_;
    std::atomic<size_t> frees_;
    free_entry *free_head_;
    element elements_[elements_count];
};

static_assert(sizeof(handle_segment) <= hal::arch_t::min_page_size);
} // namespace detail

class handle_table {
  public:
    constexpr handle_table() noexcept {}

    result<std::pair<handle_entry *, int>> allocate() noexcept {
        int fd = 0;
        auto segement = &head_segment_;
        while (segement) {
            auto handle = segement->allocate();
            if (handle.is_ok()) {
                fd += handle.unwrap().second;
                return ok(std::make_pair(handle.unwrap().first, fd));
            } else {
                segement = segement->next;
                fd += detail::handle_segment::elements_count;
            }
        }
        return err(error_code::out_of_memory);
    }

    result<void> free(int fd) noexcept {
        try_var(handle, at_with_segment(fd));
        handle.first->free(handle.second);
        return ok();
    }

    result<handle_entry *> at(int fd) noexcept {
        try_var(handle, at_with_segment(fd));
        return ok(handle.second);
    }

  private:
    result<std::pair<detail::handle_segment *, handle_entry *>> at_with_segment(int fd) noexcept {
        auto segement = &head_segment_;
        while (fd >= detail::handle_segment::elements_count) {
            segement = segement->next;
            fd -= detail::handle_segment::elements_count;
        }
        try_var(handle, segement->at(fd));
        return ok(std::make_pair(segement, handle));
    }

  private:
    detail::handle_segment head_segment_;
};
} // namespace chino::os::kernel::ob
