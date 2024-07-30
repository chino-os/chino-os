// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "error.h"
#include <string_view>
#include <system_error>
#include <type_traits>

namespace chino {
#define try_(x)                                                                                                        \
    {                                                                                                                  \
        auto v = (x);                                                                                                  \
        if (!v.is_ok())                                                                                                \
            return chino::err(std::move(v.unwrap_err()));                                                              \
    }

#define try_var(name, ...)                                                                                             \
    typename decltype((__VA_ARGS__))::value_type name;                                                                 \
    {                                                                                                                  \
        auto v = (__VA_ARGS__);                                                                                        \
        if (v.is_ok())                                                                                                 \
            name = std::move(v.unwrap());                                                                              \
        else                                                                                                           \
            return chino::err(std::move(v.unwrap_err()));                                                              \
    }

#define try_var_err(name, x, e)                                                                                        \
    typename decltype((x))::value_type name;                                                                           \
    {                                                                                                                  \
        auto v = (x);                                                                                                  \
        if (v.is_ok()) {                                                                                               \
            name = std::move(v.unwrap());                                                                              \
        } else {                                                                                                       \
            e = chino::err(std::move(v.unwrap_err()));                                                                 \
            return;                                                                                                    \
        }                                                                                                              \
    }

#define try_set(name, x)                                                                                               \
    {                                                                                                                  \
        auto v = (x);                                                                                                  \
        if (v.is_ok())                                                                                                 \
            name = std::move(v.unwrap());                                                                              \
        else                                                                                                           \
            return chino::err(std::move(v.unwrap_err()));                                                              \
    }

[[noreturn]] void fail_fast(const char *message) noexcept;

template <class T> class [[nodiscard]] result;

namespace detail {
enum class result_type { ok, err };

struct ok_t {};
inline ok_t constexpr ok_v = {};

template <class T> inline bool constexpr is_result_v = false;
template <class T> inline bool constexpr is_result_v<result<T>> = true;

template <class T, class U, class Func> class map_call_impl {
    result<U> operator()(Func &&func, T &value) noexcept { return ok(func(value)); }
};

template <class T, class Func> struct map_traits {
    using U = std::invoke_result_t<Func, T>;
    static_assert(!is_result_v<U>, "Cannot map a callback returning result, use and_then instead");
    using result_t = result<U>;

    result<U> operator()(Func &&func, T &value) noexcept {
        return map_call_impl<T, U, Func>()(std::forward<Func>(func), value);
    }
};

template <class Func> struct map_traits<void, Func> {
    using U = std::invoke_result_t<Func>;
    static_assert(!is_result_v<U>, "Cannot map a callback returning result, use and_then instead");
    using result_t = result<U>;

    result<U> operator()(Func &&func) noexcept { return map_call_impl<void, U, Func>()(std::forward<Func>(func)); }
};

template <class T, class Func> struct map_err_traits {
    using U = std::invoke_result_t<Func, std::error_condition>;
    static_assert(!is_result_v<U>, "Cannot map a callback returning result, use and_then instead");

    result<U> operator()(Func &&func, std::error_condition &value) noexcept { return err(func(value)); }
};

template <class T, class Func> struct map_err_traits;

template <class T, class Func> struct and_then_traits {
    using result_t = std::invoke_result_t<Func, T>;
    using traits_t = typename result_t::traits;
    using U = typename traits_t::ok_type;
    static_assert(is_result_v<result_t>, "Cannot then a callback not returning result, use map instead");

    result_t operator()(Func &&func, T &value) noexcept { return func(value); }
};

template <class Func> struct and_then_traits<void, Func> {
    using result_t = std::invoke_result_t<Func>;
    using traits_t = typename result_t::traits;
    using U = typename traits_t::ok_type;
    static_assert(is_result_v<result_t>, "Cannot then a callback not returning result, use map instead");

    result_t operator()(Func &&func) noexcept { return func(); }
};
} // namespace detail

template <class T> class [[nodiscard]] result {
  public:
    static_assert(!detail::is_result_v<T>, "Cannot use nested result");

    using value_type = T;

    template <class... Args>
    constexpr result(detail::ok_t, Args... args) : err_(error_code::success), ok_(std::forward<Args>(args)...) {}
    constexpr result(error_code err) noexcept : err_(err) {}

    result(const result &other) : err_(other.err_) {
        if (is_ok())
            new (&ok_) T(other.ok_);
    }

    result(result &&other) : err_(other.err_) {
        if (is_ok())
            new (&ok_) T(std::move(other.ok_));
    }

    template <class U, class = std::enable_if_t<std::is_convertible_v<U, T>>>
    result(result<U> &&other) : err_(other.err_) {
        if (is_ok())
            new (&ok_) T(std::move(other.ok_));
    }

    ~result() { destroy(); }

    result &operator=(const result &other) noexcept {
        destroy();
        err_ = other.err_;
        if (is_ok())
            new (&ok_) T(other.ok_);
        return *this;
    }

    result &operator=(result &&other) noexcept {
        destroy();
        err_ = other.err_;
        if (is_ok())
            new (&ok_) T(std::move(other.ok_));
        return *this;
    }

    constexpr bool is_ok() const noexcept { return err_ == error_code::success; }

    constexpr bool is_err() const noexcept { return !is_ok(); }

    constexpr T &unwrap() & noexcept { return ok_; }

    constexpr T &&unwrap() && noexcept { return std::move(ok_); }

    constexpr error_code &unwrap_err() noexcept { return err_; }

    constexpr T &expect(const char *message) & noexcept {
        if (is_ok())
            return ok_;
        else {
            fail_fast(message);
        }
    }

    constexpr T &&expect(const char *message) && noexcept {
        if (is_ok())
            return std::move(ok_);
        else {
            fail_fast(message);
        }
    }

    template <class Func, class Traits = detail::map_traits<T, Func>>
    constexpr typename Traits::result_t &&map(Func &&func) && noexcept {
        if (is_ok())
            return Traits()(std::forward<Func>(func), std::move(ok_));
        else
            return std::move(*this);
    }

    template <class Func, class Traits = detail::map_err_traits<T, Func>>
    constexpr typename Traits::result_t &&map_err(Func &&func) && noexcept {
        if (is_ok())
            return std::move(*this);
        else
            return Traits()(std::forward<Func>(func), err_);
    }

    template <class Func, class Traits = detail::and_then_traits<T, Func>>
    constexpr typename Traits::result_t &&and_then(Func &&func) && noexcept {
        if (is_ok())
            return Traits()(std::forward<Func>(func), ok_);
        else
            return std::move(*this);
    }

  private:
    void destroy() {
        if (is_ok())
            std::destroy_at(&ok_);
        else
            std::destroy_at(&err_);
    }

  private:
    template <class U> friend class result;
    error_code err_;
    union {
        T ok_;
    };
};

template <> class [[nodiscard]] result<void> {
  public:
    using value_type = void;

    constexpr result() noexcept : err_(error_code::success) {}
    constexpr result(error_code err) noexcept : err_(err) {}

    bool is_ok() const noexcept { return err_ == error_code::success; }
    bool is_err() const noexcept { return !is_ok(); }

    void unwrap() noexcept {}

    error_code unwrap_err() noexcept { return err_; }

    void expect(const char *message) noexcept {
        if (is_err())
            fail_fast(message);
    }

    template <class Func, class Traits = detail::map_traits<void, Func>>
    typename Traits::result_t &&map(Func &&func) && noexcept {
        if (is_ok())
            return Traits()(std::forward<Func>(func));
        else
            return std::move(*this);
    }

    template <class Func> result<void> map_err(Func &&func) && noexcept {
        if (is_ok())
            return std::move(*this);
        else
            return func(err_);
    }

    template <class Func, class Traits = detail::and_then_traits<void, Func>>
    typename Traits::result_t &&and_then(Func &&func) && noexcept {
        if (is_ok())
            return Traits()(std::forward<Func>(func));
        else
            return std::move(*this);
    }

  private:
    error_code err_;
};

inline constexpr result<void> ok() { return {}; }

template <class T, class... Args> constexpr result<T> ok(Args &&...args) {
    return {detail::ok_v, std::forward<Args>(args)...};
}

template <class T> constexpr result<std::decay_t<T>> ok(T &&value) { return {detail::ok_v, std::forward<T>(value)}; }

inline constexpr error_code err(error_code value) noexcept { return value; }

namespace detail {
template <class T, class Func> class map_call_impl<T, void, Func> {
    result<void> operator()(Func &&func, T &value) noexcept {
        func(value);
        return ok();
    }
};

template <class Func> class map_call_impl<void, void, Func> {
    result<void> operator()(Func &&func) noexcept {
        func();
        return ok();
    }
};
} // namespace detail
} // namespace chino
