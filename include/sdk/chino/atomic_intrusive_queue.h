// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "compiler.h"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <result.h>

namespace chino {
namespace detail {
class atomic_intrusive_queue_storage;
}

class atomic_intrusive_queue_node {
  public:
    constexpr atomic_intrusive_queue_node(atomic_intrusive_queue_node *next = nullptr) noexcept : next(next) {}

  private:
    friend class detail::atomic_intrusive_queue_storage;

    std::atomic<atomic_intrusive_queue_node *> next;
};

namespace detail {
class atomic_intrusive_queue_storage {
  public:
    using node_type = atomic_intrusive_queue_node;

    CHINO_NONCOPYABLE(atomic_intrusive_queue_storage);

    constexpr atomic_intrusive_queue_storage() noexcept : head_(), tail_(&dummy_), size_(0) {}

    static node_type *next(node_type *pivot) noexcept { return pivot->next.load(); }

    bool empty() const noexcept { return size() == 0; }
    size_t size() const noexcept { return size_.load(std::memory_order_acquire); }

    node_type *front() noexcept { return head().load(std::memory_order_acquire); }

    node_type *back() noexcept {
        auto back = tail_.load(std::memory_order_acquire);
        return back != &dummy_ ? back : nullptr;
    }

    void push(node_type *node) noexcept {
        tail_.load()->next.store(node, std::memory_order_relaxed);
        tail_.store(node, std::memory_order_relaxed);
        size_.fetch_add(1, std::memory_order_release);
    }

    node_type *pop() noexcept {
        auto node = head().load(std::memory_order_acquire);
        if (node) {
            head().store(node->next.load(std::memory_order_relaxed));
            size_.fetch_sub(1, std::memory_order_release);
        } else {
            tail_.store(&dummy_);
        }
        return node;
    }

  private:
    std::atomic<node_type *> &head() noexcept { return head_.next; }

  private:
    node_type dummy_;
    std::atomic<node_type *> tail_;
    std::atomic<size_t> size_;
};
} // namespace detail

template <class TContainer, intrusive_list_node TContainer::*member>
class atomic_queue : protected detail::intrusive_list_storage {
  public:
    using base_type = detail::intrusive_list_storage;
    using node_type = intrusive_list_node;

    using base_type::empty;
    using base_type::size;

    static TContainer *container_of(node_type *node) noexcept {
        return reinterpret_cast<TContainer *>(reinterpret_cast<std::byte *>(node) -
                                              reinterpret_cast<uintptr_t>(member_of(nullptr)));
    }

    static intrusive_list_node *member_of(TContainer *container) noexcept { return &(container->*member); }

    static TContainer *prev(TContainer *pivot) noexcept { return container_of(base_type::prev(member_of(pivot))); }
    static TContainer *next(TContainer *pivot) noexcept { return container_of(base_type::next(member_of(pivot))); }

    TContainer *front() noexcept { return container_of(base_type::front()); }
    TContainer *back() noexcept { return container_of(base_type::back()); }

    void insert_before(TContainer *pivot, TContainer *node) noexcept {
        base_type::insert_before(member_of(pivot), member_of(node));
    }

    void insert_after(TContainer *pivot, TContainer *node) noexcept {
        base_type::insert_after(member_of(pivot), member_of(node));
    }

    void push_front(TContainer *node) noexcept { base_type::push_front(member_of(node)); }

    void push_back(TContainer *node) noexcept { base_type::push_back(member_of(node)); }

    void remove(TContainer *node) noexcept { base_type::remove(member_of(node)); }
};
} // namespace chino
