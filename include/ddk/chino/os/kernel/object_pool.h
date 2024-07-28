// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <array>
#include <chino/os/kernel/ps.h>

namespace chino::os::kernel {
namespace detail {

struct object_entry {
    uintptr_t used;
    object_entry *next;
};

class object_segment_base {
  public:
    CHINO_NONCOPYABLE(object_segment_base);

    static constexpr size_t segment_size = hal::arch_t::min_page_size;

    constexpr object_segment_base(size_t elements_count) noexcept
        : next(nullptr), frees_(elements_count), free_head_(elements_) {}

    result<std::pair<object_entry *, size_t>> allocate() noexcept {
        std::unique_lock<decltype(lock_)> lock(lock_);
        if (free_head_) {
            auto item = reinterpret_cast<object_entry *>(free_head_);
            free_head_ = free_head_->next;
            frees_.fetch_sub(1, std::memory_order_acq_rel);
            return ok(std::make_pair(item, index(item)));
        } else {
            return err(error_code::out_of_memory);
        }
    }

    void free(object_entry *handle, void (*dtor)(object_entry *)) noexcept {
        auto head = reinterpret_cast<object_entry *>(handle);
        dtor(head);
        std::unique_lock<decltype(lock_)> lock(lock_);
        *head = {.used = 0, .next = free_head_};
        free_head_ = head;
        frees_.fetch_add(1, std::memory_order_acq_rel);
    }

    result<object_entry *> at(size_t index) noexcept {
        auto &element = elements_[index];
        if (!element.used)
            return err(error_code::invalid_argument);
        return ok(&element);
    }

  private:
    size_t index(object_entry *handle) const noexcept { return handle - elements_; }

  public:
    object_segment_base *next;

  private:
    ps::irq_spin_lock lock_;
    std::atomic<size_t> frees_;
    object_entry *free_head_;
    object_entry elements_[0];
};

class object_pool_impl {
  public:
    CHINO_NONCOPYABLE(object_pool_impl);

    constexpr object_pool_impl() noexcept {}

    result<std::pair<object_entry *, size_t>> allocate(size_t elements_per_segment) noexcept {
        size_t index = 0;
        auto segement = head_segment();
        while (segement) {
            auto entry = segement->allocate();
            if (entry.is_ok()) {
                index += entry.unwrap().second;
                return ok(std::make_pair(entry.unwrap().first, index));
            } else {
                segement = segement->next;
                index += elements_per_segment;
            }
        }
        return err(error_code::out_of_memory);
    }

    result<void> free(object_entry *entry, void (*dtor)(object_entry *)) noexcept {
        try_var(segment, find_segment(entry));
        segment->free(entry, dtor);
        return ok();
    }

    result<void> free(size_t index, void (*dtor)(object_entry *), size_t elements_per_segment) noexcept {
        try_var(entry, at_with_segment(index, elements_per_segment));
        entry.first->free(entry.second, dtor);
        return ok();
    }

    result<object_entry *> at(size_t index, size_t elements_per_segment) noexcept {
        try_var(entry, at_with_segment(index, elements_per_segment));
        return ok(entry.second);
    }

  private:
    result<std::pair<object_segment_base *, object_entry *>> at_with_segment(size_t index,
                                                                             size_t elements_per_segment) noexcept {
        auto segement = head_segment();
        while (index >= elements_per_segment) {
            segement = segement->next;
            index -= elements_per_segment;
            if (!segement)
                return err(error_code::not_found);
        }
        try_var(entry, segement->at(index));
        return ok(std::make_pair(segement, entry));
    }

    result<object_segment_base *> find_segment(object_entry *entry) noexcept {
        auto segement = head_segment();
        while (true) {
            auto entry_base = reinterpret_cast<uintptr_t>(entry);
            auto seg_base = reinterpret_cast<uintptr_t>(segement);
            if (entry_base >= seg_base + +sizeof(object_segment_base) &&
                entry_base < seg_base + object_segment_base::segment_size)
                return ok(segement);

            segement = segement->next;
            if (!segement)
                break;
        }
        return err(error_code::not_found);
    }

    object_segment_base *head_segment() noexcept {
        return reinterpret_cast<object_segment_base *>(reinterpret_cast<uintptr_t>(this));
    }
};
} // namespace detail

template <class T> class object_pool : private detail::object_pool_impl {
    class object_pool_object : public ref_counted_object<T> {
      protected:
        virtual void internal_release() noexcept;
    };

    using entry_type = std::conditional_t<Object<T>, object_pool_object, T>;

    union element {
        detail::object_entry free;
        entry_type entry;

        constexpr element() noexcept : free{} {}
        ~element() {
            if (free.used) {
                std::destroy_at(&entry);
            }
        }
    };

    class object_segment : public detail::object_segment_base {
      public:
        static constexpr size_t elements_count =
            (object_segment_base::segment_size - sizeof(object_segment_base)) / sizeof(element);

        constexpr object_segment() noexcept : object_segment_base(elements_count) {
            for (size_t i = 0; i < elements_count - 1; i++) {
                elements_[i].free = {.used = 0, .next = &elements_[i + 1].free};
            }
        }

      private:
        element elements_[elements_count];
    };

  public:
    CHINO_NONCOPYABLE(object_pool);

    constexpr object_pool() noexcept {}

    template <class... TArgs> result<std::pair<T *, size_t>> allocate(TArgs &&...args) noexcept {
        try_var(r, object_pool_impl::allocate(object_segment::elements_count));
        auto obj = reinterpret_cast<T *>(r.first);
        std::construct_at(obj, std::forward<TArgs>(args)...);
        return ok(std::make_pair(obj, r.second));
    }

    result<void> free(T *object) noexcept {
        return object_pool_impl::free(reinterpret_cast<detail::object_entry *>(object), &std::destroy_at);
    }

    result<void> free(size_t index) noexcept {
        return object_pool_impl::free(index, &std::destroy_at, object_segment::elements_count);
    }

    result<T *> at(size_t index) noexcept {
        try_var(obj, object_pool_impl::at(index, object_segment::elements_count));
        return ok(reinterpret_cast<T *>(obj));
    }

  private:
    object_segment head_segment_;
};
} // namespace chino::os::kernel
