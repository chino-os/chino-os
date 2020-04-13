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
#include <stddef.h>
#include <stdint.h>

namespace chino::arch
{
struct bits_range
{
    uint32_t start;
    uint32_t end;
};

namespace details
{
    template <class T>
    class reg_impl
    {
        static_assert(sizeof(T) == sizeof(uint32_t));
    private:
        typedef union {
            T reg;
            uint32_t data;
        } storage_t;

    public:
        T reg() const volatile noexcept
        {
            storage_t s { .data = storage_.data };
            return s.reg;
        }
        
        volatile T &reg_mut() volatile noexcept
        {
            return storage_.reg;
        }

        void reg(const T &reg) volatile noexcept
        {
            storage_t s { .reg = reg };
            storage_.data = s.data;
        }

        uint32_t raw() const volatile noexcept
        {
            return storage_.data;
        }

        void raw(uint32_t data) volatile noexcept
        {
            storage_.data = data;
        }
        
        uint32_t bits(uint32_t mask) const volatile noexcept
        {
            return raw() & mask;
        }
        
        uint32_t bits(uint32_t mask, uint32_t value) const volatile noexcept
        {
            auto old = raw();
            raw((old & ~mask) | (value & mask));
            return old;
        }

        uint32_t bits(bits_range range) const volatile noexcept
        {
            auto high_mask = (1U << range.end) - 1;
            return (raw() & high_mask) >> range.start;
        }

        uint32_t bits(bits_range range, uint32_t value) volatile noexcept
        {
            auto mask = (((1U << range.end) - 1) >> range.start) << range.start;
            auto old = raw();
            raw((old & ~mask) | ((value << range.start) & mask));
            return (old & mask) >> range.start;
        }
        
        uint32_t set(uint32_t mask) volatile noexcept
        {
            auto old = raw();
            raw(old | mask);
            return old;
        }
        
        uint32_t clear(uint32_t mask) volatile noexcept
        {
            auto old = raw();
            raw(old & ~mask);
            return old;
        }

    private:
        storage_t storage_;
    };
}

template <class T>
using reg_t = details::reg_impl<T>;
} // namespace chino::arch
