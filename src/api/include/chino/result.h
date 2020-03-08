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
#include <optional>
#include <type_traits>
#include <variant>

namespace chino
{
template <class T>
class Ok
{
public:
    template <class U = T>
    constexpr Ok(U &&value)
        : value_(std::forward<U>(value)) {}

    template <class... Args>
    constexpr explicit Ok(std::in_place_t, Args &&... args)
        : value_(std::forward<Args>(args)...) {}

    constexpr const T &get() const noexcept
    {
        return value_;
    }

    constexpr T &get() noexcept
    {
        return value_;
    }

private:
    T value_;
};

template <>
class Ok<void>
{
public:
};

template <class E>
class Err
{
public:
    template <class U = E>
    constexpr Err(U &&value)
        : err_(std::forward<U>(value)) {}

    template <class... Args>
    constexpr explicit Err(std::in_place_t, Args &&... args)
        : err_(std::forward<Args>(args)...) {}

private:
    E err_;
};

template <class T, class E>
class [[nodiscard]] result
{
public:
    constexpr result(Ok<T> && value)
        : ok_or_err_(std::move(value)) {}

    constexpr result(Err<E> && err)
        : ok_or_err_(std::move(err)) {}

    constexpr bool is_ok() const noexcept { return ok_or_err_.index() == 0; }
    constexpr bool is_err() const noexcept { return ok_or_err_.index() == 1; }

private:
    std::variant<Ok<T>, Err<E>> ok_or_err_;
};

inline constexpr Ok<void> ok()
{
    return {};
}

template <class T, class... Args>
constexpr Ok<T> ok(Args &&... args)
{
    return Ok<T>(std::in_place, std::forward<Args>(args)...);
}

template <class T>
constexpr Ok<std::decay_t<T>> ok(T &&value)
{
    return Ok<std::decay_t<T>>(std::forward<T>(value));
}

template <class E, class... Args>
constexpr Err<E> err(Args &&... args)
{
    return Err<E>(std::in_place, std::forward<Args>(args)...);
}

template <class E>
constexpr Err<std::decay_t<E>> err(E &&value)
{
    return Err<std::decay_t<E>>(std::forward<E>(value));
}
}
