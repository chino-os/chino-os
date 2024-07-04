// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "chino/error.h"
#include "object_kind.h"
#include <chino/compiler.h>
#include <chino/result.h>
#include <type_traits>

namespace chino::os::kernel {
#define CHINO_DEFINE_KERNEL_OBJECT_KIND(base_t, kind_)                                                                 \
  public:                                                                                                              \
    static constexpr object_kind kind() noexcept { return kind_; }                                                     \
    const object_kind &runtime_kind() const noexcept override { return kind_; }                                        \
                                                                                                                       \
  protected:                                                                                                           \
    bool is_a(const object_kind &kind) const noexcept override { return kind == kind_ || base_t::is_a(kind); }

class object;

template <class T>
concept Object = std::is_same_v<T, object> || std::is_base_of_v<object, T>;

class object {
  public:
    CHINO_NONCOPYABLE(object);

    constexpr object() noexcept : ref_count_(1) {}
    virtual ~object() = default;

    void operator delete(void *) {}

    /** @brief Get the kind of the object */
    virtual const object_kind &runtime_kind() const noexcept = 0;

  protected:
    template <Object T> friend class object_ptr;

    /** @brief Is the object an instance of specific kind */
    virtual bool is_a(const object_kind &kind) const noexcept;

  private:
    uint32_t add_ref() const noexcept { return ref_count_.fetch_add(1, std::memory_order_relaxed); }

    uint32_t release() const noexcept {
        auto count = ref_count_.fetch_sub(1, std::memory_order_acq_rel);
        if (count == 1) {
            delete this;
        }
        return count;
    }

  private:
    mutable std::atomic<uint32_t> ref_count_;
};

template <Object T> class object_ptr {
  public:
    using object_type = T;

    /** @brief Construct an empty object */
    constexpr object_ptr(std::nullptr_t = nullptr) noexcept : object_(nullptr) {}
    ~object_ptr() { release(); }

    object_ptr(T *node) noexcept : object_(node) { add_ref(); }
    object_ptr(std::in_place_t, T *node) noexcept : object_(node) {}

    object_ptr(object_ptr &&other) noexcept : object_(other.object_) { other.object_ = nullptr; }

    object_ptr(const object_ptr &other) noexcept : object_(other.object_) { add_ref(); }

    template <Object U, class = std::enable_if_t<std::is_convertible_v<U *, T *>>>
    object_ptr(object_ptr<U> &&other) noexcept : object_(other.object_) {
        other.object_ = nullptr;
    }

    template <Object U, class = std::enable_if_t<std::is_convertible_v<U *, T *>>>
    object_ptr(const object_ptr<U> &other) noexcept : object_(other.object_) {
        add_ref();
    }

    template <class... TArgs>
    object_ptr(std::in_place_t, TArgs &&...args) : object_(new T(std::forward<TArgs>(args)...)) {}

    /** @brief Get the managed pointer to the object */
    T *get() const noexcept { return object_; }
    T *operator->() const noexcept { return get(); }

    bool empty() const noexcept { return !object_; }

    object_ptr value_or(object_ptr &&other) const noexcept {
        if (!empty())
            return *this;
        return std::move(other);
    }

    /** @brief Is the object an instance of specific type */
    bool is_a(const object_kind &kind) const noexcept { return object_ && static_cast<object *>(object_)->is_a(kind); }

    /** @brief Is the object an instance of specific type */
    template <class U> bool is_a() const noexcept { return is_a(U::object_type::kind()); }

    template <class U> result<U> as() const noexcept {
        if (is_a<U>()) {
            return ok(U(static_cast<typename U::object_type *>(object_)));
        } else {
            return err(error_code::bad_cast);
        }
    }

    object_ptr &operator=(object_ptr &&other) noexcept {
        if (this != &other) {
            release();
            object_ = other.object_;
            other.object_ = nullptr;
        }
        return *this;
    }

    object_ptr &operator=(const object_ptr &other) noexcept {
        if (this != &other) {
            release();
            object_ = other.object_;
            add_ref();
        }
        return *this;
    }

    T *detach() noexcept {
        auto obj = object_;
        object_ = nullptr;
        return obj;
    }

    T **release_and_addressof() noexcept {
        release();
        return &object_;
    }

    void dangerous_add_ref() noexcept { return add_ref(); }

  private:
    void add_ref() noexcept {
        if (object_) {
            object_->add_ref();
        }
    }

    void release() noexcept {
        auto obj = object_;
        if (obj) {
            obj->release();
            object_ = nullptr;
        }
    }

  private:
    template <Object U> friend class object_ptr;

    T *object_;
};
} // namespace chino::os::kernel
