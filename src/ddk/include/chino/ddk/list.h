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
#include <cassert>
#include <chino/ddk/ps.h>
#include <cstddef>
#include <cstdint>
#include <mutex>

namespace chino
{
inline constexpr size_t VOID_LIST_NODE_SIZE = sizeof(uintptr_t) * 3;

template <class TOwner, class TValue, ptrdiff_t OwnerOffset>
class list;

template <class TOwner, class TValue, ptrdiff_t OwnerOffset>
struct list_node
{
    using list_t = list<TOwner, TValue, OwnerOffset>;

    list_t *l;
    list_node *prev;
    list_node *next;
    TValue value;

    constexpr list_node() noexcept
        : l(nullptr), prev(nullptr), next(nullptr) {}

    TOwner *owner() const noexcept
    {
        auto addr = reinterpret_cast<ptrdiff_t>(this) - OwnerOffset;
        return reinterpret_cast<TOwner *>(addr);
    }
};

template <class TOwner, ptrdiff_t OwnerOffset>
struct list_node<TOwner, void, OwnerOffset>
{
    using list_t = list<TOwner, void, OwnerOffset>;

    list_t *l;
    list_node *prev;
    list_node *next;

    constexpr list_node() noexcept
        : l(nullptr), prev(nullptr), next(nullptr) {}

    TOwner *owner() const noexcept
    {
        auto addr = reinterpret_cast<ptrdiff_t>(this) - OwnerOffset;
        return reinterpret_cast<TOwner *>(addr);
    }
};

template <class TOwner, class TValue, ptrdiff_t OwnerOffset>
class list
{
public:
    using node_t = list_node<TOwner, TValue, OwnerOffset>;

    constexpr list() noexcept
        : head_(nullptr), tail_(nullptr) {}

    void add_first_nolock(node_t *node) noexcept
    {
        assert(!node->l);
        node->l = this;
        node->prev = nullptr;
        if (head_)
        {
            node->next = head_;
            head_->prev = node;
            head_ = node;
        }
        else
        {
            node->next = nullptr;
            head_ = tail_ = node;
        }
    }

    void add_first(node_t *node) noexcept
    {
        std::unique_lock lock(lock_);
        add_first_nolock(node);
    }

    void add_last_nolock(node_t *node) noexcept
    {
        assert(!node->l);
        node->l = this;
        node->next = nullptr;
        if (tail_)
        {
            node->prev = tail_;
            tail_->next = node;
            tail_ = node;
        }
        else
        {
            node->prev = nullptr;
            head_ = tail_ = node;
        }
    }

    void add_last(node_t *node) noexcept
    {
        std::unique_lock lock(lock_);
        add_last_nolock(node);
    }

    void remove(node_t *node) noexcept
    {
        std::unique_lock lock(lock_);
        assert(node->l == this);
        node->l = nullptr;
        if (node->prev)
            node->prev->next = node->next;
        if (node->next)
            node->next->prev = node->prev;
        if (head_ == node)
            head_ = node->next;
        if (tail_ == node)
            tail_ = node->prev;

        node->prev = node->next = nullptr;
    }

    void insert_before_nolock(node_t *offset, node_t *node) noexcept
    {
        if (!offset)
        {
            add_last_nolock(node);
        }
        else
        {
            auto prev = offset->prev;
            assert(!node->l);
            node->l = this;
            node->prev = prev;
            node->next = offset;
            if (prev)
                prev->next = node;
            offset->prev = node;

            if (offset == head_)
                head_ = node;
        }
    }

    node_t *first_nolock() noexcept { return head_; }
    threading::irq_spinlock &syncroot() noexcept { return lock_; }

private:
    threading::irq_spinlock lock_;
    node_t *head_;
    node_t *tail_;
};

#ifndef list_t_of_node
#define list_t_of_node(member) typename decltype(member)::list_t
#endif
}
