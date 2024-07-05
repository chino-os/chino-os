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
class atomic_queue_storage;
}

class atomic_queue_node {
  public:
    constexpr atomic_queue_node() noexcept : next(nullptr) {}

  private:
    friend class detail::atomic_queue_storage;

    std::atomic<atomic_queue_node *> next;
};

namespace detail {
class atomic_queue_storage {
  public:
    using node_type = atomic_queue_node;

    CHINO_NONCOPYABLE(atomic_queue_storage);

    constexpr atomic_queue_storage() noexcept : head_(nullptr), tail_(nullptr), size_(0) {}

    static node_type *next(node_type *pivot) noexcept { return pivot->next.load(std::memory_order_relaxed); }

    bool empty() const noexcept { return size() == 0; }
    size_t size() const noexcept { return size_.load(std::memory_order_relaxed); }

    node_type *front() noexcept { return head_.load(std::memory_order_relaxed); }

    void push(node_type *node) noexcept {
        node->prev = tail_;
        if (tail_)
            tail_->next = node;
        else
            head_ = tail_ = node;
        size_++;
    }

    result<node_type *> try_pop() noexcept {}

  private:
    std::atomic<node_type *> head_;
    std::atomic<node_type *> tail_;
    std::atomic<size_t> size_;
};
} // namespace detail

template <class TContainer, intrusive_list_node TContainer::*member>
class intrusive_list : protected detail::intrusive_list_storage {
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
