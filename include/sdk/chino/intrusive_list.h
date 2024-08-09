// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "compiler.h"
#include <cstddef>
#include <cstdint>

namespace chino {
namespace detail {
class intrusive_list_storage;
}

class intrusive_list_node {
  public:
    constexpr intrusive_list_node(intrusive_list_node *prev = nullptr, intrusive_list_node *next = nullptr) noexcept
        : prev_(prev), next_(next) {}

    bool in_list() const noexcept { return prev_; }

  private:
    friend class detail::intrusive_list_storage;

    intrusive_list_node *prev_;
    intrusive_list_node *next_;
};

namespace detail {
class intrusive_list_storage {
  public:
    using node_type = intrusive_list_node;

    CHINO_NONCOPYABLE(intrusive_list_storage);

    constexpr intrusive_list_storage() noexcept : dummy_(), size_(0) {
        dummy_.prev_ = &dummy_;
        dummy_.next_ = &dummy_;
    }

    node_type *prev(node_type *pivot) noexcept { return pivot->prev_ != &dummy_ ? pivot->prev_ : nullptr; }
    node_type *next(node_type *pivot) noexcept { return pivot->next_ != &dummy_ ? pivot->next_ : nullptr; }

    constexpr bool empty() const noexcept { return size() == 0; }
    constexpr size_t size() const noexcept { return size_; }

    node_type *front() noexcept { return head() != &dummy_ ? head() : nullptr; }
    node_type *back() noexcept { return tail() != &dummy_ ? tail() : nullptr; }

    void insert_after(node_type *pivot, node_type *node) noexcept {
        node->next_ = pivot->next_;
        node->prev_ = pivot;
        pivot->next_->prev_ = node;
        pivot->next_ = node;
        size_++;
    }

    void insert_before(node_type *pivot, node_type *node) noexcept {
        node->prev_ = pivot->prev_;
        node->next_ = pivot;
        pivot->prev_->next_ = node;
        pivot->prev_ = node;
        size_++;
    }

    void push_front(node_type *node) noexcept { insert_after(&dummy_, node); }
    void push_back(node_type *node) noexcept { insert_before(&dummy_, node); }

    void remove(node_type *node) noexcept {
        node->next_->prev_ = node->prev_;
        node->prev_->next_ = node->next_;
        node->prev_ = node->next_ = nullptr;
        size_--;
    }

  private:
    node_type *head() noexcept { return dummy_.next_; }
    node_type *tail() noexcept { return dummy_.prev_; }

  private:
    node_type dummy_;
    size_t size_;
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

    TContainer *prev(TContainer *pivot) noexcept {
        auto prev = base_type::prev(member_of(pivot));
        return prev ? container_of(prev) : nullptr;
    }

    TContainer *next(TContainer *pivot) noexcept {
        auto next = base_type::next(member_of(pivot));
        return next ? container_of(next) : nullptr;
    }

    TContainer *front() noexcept {
        auto next = base_type::front();
        return next ? container_of(next) : nullptr;
    }

    TContainer *back() noexcept {
        auto next = base_type::back();
        return next ? container_of(next) : nullptr;
    }

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
