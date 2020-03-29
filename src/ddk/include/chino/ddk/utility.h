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
#include <type_traits>

namespace chino
{
// clang-format off
#define CHINO_CONCAT_(a, b) a##b
#define CHINO_CONCAT(a, b) CHINO_CONCAT_(a,b)
// clang-format on

template <class T, class U, class = std::enable_if_t<std::is_integral_v<T> && std::is_integral_v<U>>>
constexpr T align(T value, U alignment) noexcept
{
    auto rem = value % alignment;
    return rem ? value + (alignment - rem) : value;
}

template <class T, class U, class = std::enable_if_t<std::is_integral_v<T> && std::is_integral_v<U>>>
constexpr T ceil_div(T numerator, U denominator) noexcept
{
    return (numerator + (denominator - 1)) / denominator;
}
}
