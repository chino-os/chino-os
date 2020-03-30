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
#include <string_view>
#include <type_traits>
#include <variant>

namespace chino
{
[[noreturn]] void panic(std::string_view message) noexcept;

#define try_(x)                    \
    {                              \
        auto v = (x);              \
        if (!v.is_ok())            \
            return v.unwrap_err(); \
    }

#define try_var(name, x)                          \
    typename decltype((x))::traits::ok_type name; \
    {                                             \
        auto v = (x);                             \
        if (v.is_ok())                            \
            name = v.unwrap();                    \
        else                                      \
            return v.unwrap_err();                \
    }

#define try_set(name, x)           \
    {                              \
        auto v = (x);              \
        if (v.is_ok())             \
            name = v.unwrap();     \
        else                       \
            return v.unwrap_err(); \
    }

template <class T>
struct Ok
{
    constexpr Ok(T &&value)
        : value(std::move(value)) {}

    constexpr Ok(const T &value)
        : value(value) {}

    template <class... Args>
    constexpr explicit Ok(std::in_place_t, Args &&... args)
        : value(std::forward<Args>(args)...) {}

    T value;
};

template <>
struct Ok<void>
{
};

template <class E>
struct Err
{
    constexpr Err(E &&value)
        : err(std::move(value)) {}

    constexpr Err(const E &value)
        : err(value) {}

    template <class... Args>
    constexpr explicit Err(std::in_place_t, Args &&... args)
        : err(std::forward<Args>(args)...) {}

    E err;
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

template <class T, class E>
class [[nodiscard]] result;

namespace details
{
    template <class T>
    inline bool constexpr is_result_v = false;
    template <class T, class E>
    inline bool constexpr is_result_v<result<T, E>> = true;

    template <class T, class E>
    struct result_traits
    {
        static_assert(!is_result_v<T>, "Cannot use nested result");

        using ok_type = T;
        using err_type = E;
    };

    template <class T, class E, class Func>
    struct map_traits
    {
        using U = std::invoke_result_t<Func, T>;
        static_assert(!is_result_v<U>, "Cannot map a callback returning result, use and_then instead");
        using result_t = result<U, E>;

        result_t operator()(Func &&func, Ok<T> &value) noexcept
        {
            if constexpr (std::is_same_v<U, void>)
            {
                func(value.value);
                return ok();
            }
            else
            {
                return ok(func(value.value));
            }
        }
    };

    template <class E, class Func>
    struct map_traits<void, E, Func>
    {
        using U = std::invoke_result_t<Func>;
        static_assert(!is_result_v<U>, "Cannot map a callback returning result, use and_then instead");
        using result_t = result<U, E>;

        result_t operator()(Func &&func, Ok<void> &value) noexcept
        {
            if constexpr (std::is_same_v<U, void>)
            {
                func();
                return ok();
            }
            else
            {
                return ok(func());
            }
        }
    };

    template <class T, class E, class Func>
    struct map_err_traits
    {
        using U = std::invoke_result_t<Func, E>;
        static_assert(!is_result_v<U>, "Cannot map a callback returning result, use and_then instead");
        using result_t = result<T, U>;

        result_t operator()(Func &&func, Err<E> &value) noexcept
        {
            return err(func(value.err));
        }
    };

    template <class T, class E, class Func>
    struct and_then_traits
    {
        using result_t = std::invoke_result_t<Func, T>;
        using traits_t = typename result_t::traits;
        using U = typename traits_t::ok_type;
        static_assert(is_result_v<result_t>, "Cannot then a callback not returning result, use map instead");
        static_assert(std::is_same_v<E, typename traits_t::err_type>, "Callback should return same err type");

        result_t operator()(Func &&func, Ok<T> &value) noexcept
        {
            return func(value.value);
        }
    };

    template <class E, class Func>
    struct and_then_traits<void, E, Func>
    {
        using result_t = std::invoke_result_t<Func>;
        using traits_t = typename result_t::traits;
        using U = typename traits_t::ok_type;
        static_assert(is_result_v<result_t>, "Cannot then a callback not returning result, use map instead");
        static_assert(std::is_same_v<E, typename traits_t::err_type>, "Callback should return same err type");

        result_t operator()(Func &&func, Ok<void> &value) noexcept
        {
            return func();
        }
    };
}

template <class T, class E>
class [[nodiscard]] result
{
public:
    using traits = details::result_traits<T, E>;

    constexpr result(Ok<T> value)
        : ok_or_err_(std::move(value)) {}

    constexpr result(Err<E> err)
        : ok_or_err_(std::move(err)) {}

    constexpr bool is_ok() const noexcept { return ok_or_err_.index() == 0; }
    constexpr bool is_err() const noexcept { return ok_or_err_.index() == 1; }

    constexpr auto unwrap() noexcept
    {
        if (is_ok())
        {
            if constexpr (std::is_same_v<T, void>)
                return;
            else
                return value().value;
        }
        else
        {
            panic(std::string_view {});
        }
    }

    constexpr E &unwrap_err() noexcept
    {
        if (is_ok())
        {
            panic(std::string_view {});
        }
        else
        {
            return err().err;
        }
    }

    constexpr auto expect(std::string_view message) noexcept
    {
        if (is_ok())
        {
            if constexpr (std::is_same_v<T, void>)
                return;
            else
                return std::ref(value().value);
        }
        else
        {
            panic(message);
        }
    }

    template <class Func, class Traits = details::map_traits<T, E, Func>>
    constexpr typename Traits::result_t map(Func && func) noexcept
    {
        if (is_ok())
            return Traits()(std::forward<Func>(func), value());
        else
            return err();
    }

    template <class Func, class Traits = details::map_err_traits<T, E, Func>>
    constexpr typename Traits::result_t map_err(Func && func) noexcept
    {
        if (is_ok())
            return value();
        else
            return Traits()(std::forward<Func>(func), err());
    }

    template <class Func, class Traits = details::and_then_traits<T, E, Func>>
    constexpr typename Traits::result_t and_then(Func && func) noexcept
    {
        if (is_ok())
            return Traits()(std::forward<Func>(func), value());
        else
            return err();
    }

private:
    constexpr Ok<T> &value() noexcept { return std::get<Ok<T>>(ok_or_err_); }
    constexpr Err<E> &err() noexcept { return std::get<Err<E>>(ok_or_err_); }

private:
    std::variant<Ok<T>, Err<E>> ok_or_err_;
};
}
