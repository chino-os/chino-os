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
#include <cstddef>
#include <cstdint>

namespace chino
{
template <class TOwner, class TValue, ptrdiff_t OwnerOffset>
class list;

template <class TOwner, class TValue, ptrdiff_t OwnerOffset>
struct list_node
{
    using list_t = list<TOwner, TValue, OwnerOffset>;

    list_node *prev;
    list_node *next;
    TValue value;

    constexpr list_node() noexcept
        : prev(nullptr), next(nullptr) {}

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

    list_node *prev;
    list_node *next;

    constexpr list_node() noexcept
        : prev(nullptr), next(nullptr) {}

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

private:
    node_t *head_;
    node_t *tail_;
};

#define list_t_of_node(member) typename decltype(member)::list_t
}
