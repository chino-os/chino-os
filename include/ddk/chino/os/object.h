// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "chino/error.h"
#include "object_kind.h"
#include <chino/compiler.h>
#include <chino/intrusive_list.h>
#include <chino/result.h>
#include <type_traits>

namespace chino::os {
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
    constexpr object() noexcept = default;
    virtual ~object() = default;

    virtual uint32_t add_ref() noexcept { return 1; }
    virtual uint32_t dec_ref() noexcept { return 1; }

    void operator delete(void *) {}

    /** @brief Get the kind of the object */
    virtual const object_kind &runtime_kind() const noexcept = 0;

    /** @brief Is the object an instance of specific kind */
    virtual bool is_a(const object_kind &kind) const noexcept;
};

template <class T> class object_ptr {
  public:
    using object_type = T;

    /** @brief Construct an empty object */
    constexpr object_ptr(std::nullptr_t = nullptr) noexcept : object_(nullptr) {}
    ~object_ptr() { dec_ref(); }

    object_ptr(T *node) noexcept : object_(node) { add_ref(); }
    object_ptr(std::in_place_t, T *node) noexcept : object_(node) {}

    object_ptr(object_ptr &&other) noexcept : object_(other.object_) { other.object_ = nullptr; }

    object_ptr(const object_ptr &other) noexcept : object_(other.object_) { add_ref(); }

    template <class U, class = std::enable_if_t<std::is_convertible_v<U *, T *>>>
    object_ptr(object_ptr<U> &&other) noexcept : object_(other.object_) {
        other.object_ = nullptr;
    }

    template <class U, class = std::enable_if_t<std::is_convertible_v<U *, T *>>>
    object_ptr(const object_ptr<U> &other) noexcept : object_(other.object_) {
        add_ref();
    }

    template <class... TArgs>
    object_ptr(std::in_place_t, TArgs &&...args) : object_(new T(std::forward<TArgs>(args)...)) {}

    /** @brief Get the managed pointer to the object */
    T *get() const noexcept { return object_; }
    T &operator*() const noexcept { return *get(); }
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
    template <class U> bool is_a() const noexcept { return is_a(U::kind()); }

    template <class U> result<object_ptr<U>> as() const noexcept {
        if (is_a<U>()) {
            return ok(static_cast<U *>(object_));
        } else {
            return err(error_code::bad_cast);
        }
    }

    object_ptr &operator=(object_ptr &&other) noexcept {
        if (this != &other) {
            dec_ref();
            object_ = other.object_;
            other.object_ = nullptr;
        }
        return *this;
    }

    object_ptr &operator=(const object_ptr &other) noexcept {
        if (this != &other) {
            dec_ref();
            object_ = other.object_;
            add_ref();
        }
        return *this;
    }

    T *release() noexcept {
        auto obj = object_;
        object_ = nullptr;
        return obj;
    }

    T **dec_ref_and_addressof() noexcept {
        dec_ref();
        return &object_;
    }

  private:
    void add_ref() noexcept {
        if (object_) {
            object_->add_ref();
        }
    }

    void dec_ref() noexcept {
        auto obj = object_;
        if (obj) {
            obj->dec_ref();
            object_ = nullptr;
        }
    }

  private:
    template <class U> friend class object_ptr;

    T *object_;
};

template <class T> class lazy_construct {
  public:
    CHINO_NONCOPYABLE(lazy_construct);

    constexpr lazy_construct() noexcept : storage_{} {}

    template <class... TArgs> void construct(TArgs &&...args) noexcept {
        std::construct_at(get(), std::forward<TArgs>(args)...);
    }

    T *get() noexcept { return reinterpret_cast<T *>(storage_); }
    T &operator*() noexcept { return *get(); }
    T *operator->() const noexcept { return get(); }
    T *operator->() noexcept { return get(); }

  private:
    alignas(alignof(T)) std::byte storage_[sizeof(T)];
};

template <class T> class ref_counted_object : public T {
  public:
    using T::T;

    uint32_t add_ref() noexcept override { return ref_count_.fetch_add(1, std::memory_order_relaxed); }

    uint32_t dec_ref() noexcept override {
        auto count = ref_count_.fetch_sub(1, std::memory_order_acq_rel);
        if (count == 1) {
            internal_release();
        }
        return count;
    }

  protected:
    virtual void internal_release() noexcept = 0;

  private:
    std::atomic<uint32_t> ref_count_ = 1;
};
} // namespace chino::os
