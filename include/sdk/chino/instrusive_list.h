// MIT License
//
// Copyright (c) 2020 SunnyCase
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
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
    constexpr intrusive_list_node() noexcept : prev(nullptr), next(nullptr) {}

  private:
    friend class detail::intrusive_list_storage;

    intrusive_list_node *prev;
    intrusive_list_node *next;
};

namespace detail {
class intrusive_list_storage {
  public:
    using node_type = intrusive_list_node;

    CHINO_NONCOPYABLE(intrusive_list_storage);

    constexpr intrusive_list_storage() noexcept : head_(nullptr), tail_(nullptr), size_(0) {}

    bool empty() const noexcept { return size() == 0; }
    size_t size() const noexcept { return size_; }

    void insert_before(node_type *pivot, node_type *node) noexcept {
        node->prev = pivot->prev;
        node->next = pivot;
        if (pivot->prev)
            pivot->prev->next = node;
        else
            head_ = node;
        pivot->prev = node;
        size_++;
    }

    void insert_after(node_type *pivot, node_type *node) noexcept {
        node->prev = pivot;
        node->next = pivot->next;
        if (pivot->next)
            pivot->next->prev = node;
        else
            tail_ = node;
        pivot->next = node;
        size_++;
    }

    void insert_head(node_type *node) noexcept {
        node->next = head_;
        if (head_)
            head_->prev = node;
        else
            head_ = tail_ = node;
        size_++;
    }

    void insert_tail(node_type *node) noexcept {
        node->prev = tail_;
        if (tail_)
            tail_->next = node;
        else
            head_ = tail_ = node;
        size_++;
    }

    void remove(node_type *node) noexcept { (void)node; }

  private:
    node_type *head_;
    node_type *tail_;
    size_t size_;
};
} // namespace detail

template <class TContainer, intrusive_list_node TContainer::*member>
class intrusive_list : protected detail::intrusive_list_storage {
  public:
    using base_type = detail::intrusive_list_storage;
    using node_type = intrusive_list_node;

    static intrusive_list_node *member_of(TContainer *container) noexcept { return &(container->*member); }

    void insert_before(TContainer *pivot, TContainer *node) noexcept {
        base_type::insert_before(member_of(pivot), member_of(node));
    }

    void insert_after(TContainer *pivot, TContainer *node) noexcept {
        base_type::insert_after(member_of(pivot), member_of(node));
    }

    void insert_head(TContainer *node) noexcept { base_type::insert_head(node); }
};
} // namespace chino
