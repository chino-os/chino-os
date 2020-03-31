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
#include <limits>

namespace chino
{
#ifndef DEFINE_ENUM_FLAG_OPERATORS
#define DEFINE_ENUM_FLAG_OPERATORS(ENUMTYPE)                                                              \
    inline ENUMTYPE operator|(ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE(((int)a) | ((int)b)); }           \
    inline ENUMTYPE &operator|=(ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((int &)a) |= ((int)b)); } \
    inline ENUMTYPE operator&(ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE(((int)a) & ((int)b)); }           \
    inline ENUMTYPE &operator&=(ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((int &)a) &= ((int)b)); } \
    inline ENUMTYPE operator~(ENUMTYPE a) { return ENUMTYPE(~((int)a)); }                                 \
    inline ENUMTYPE operator^(ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE(((int)a) ^ ((int)b)); }           \
    inline ENUMTYPE &operator^=(ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((int &)a) ^= ((int)b)); }
#endif

inline constexpr size_t MAX_OBJECT_NAME = 15;
inline constexpr char DIRECTORY_SEPARATOR = '/';

typedef struct _handle
{
    uint16_t value;

    static constexpr _handle invalid() { return { std::numeric_limits<uint16_t>::max() }; }
} handle_t;

static constexpr bool operator==(const _handle &lhs, const _handle &rhs) noexcept
{
    return lhs.value == rhs.value;
}

static constexpr bool operator!=(const _handle &lhs, const _handle &rhs) noexcept
{
    return lhs.value != rhs.value;
}

enum class access_mask : uint32_t
{
    none = 0,
    generic_all = 0b001,
    generic_read = 0b010,
    generic_write = 0b100
};
DEFINE_ENUM_FLAG_OPERATORS(access_mask);
}
