// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "../compiler.h"
#include "../result.h"
#include <atomic>
#include <chrono>
#include <optional>

namespace chino::os {
enum class thread_priority { idle = 0, lowest = 1, low = 2, normal = 3, high = 4, highest = 5, max = highest };
enum class thread_status { ready = 0, running = 1, blocked = 2, delayed = 3, terminated = 4, max = terminated };

typedef int (*thread_start_t)(void *);

result<void> atomic_wait(std::atomic<uint32_t> &atomic, uint32_t old,
                         std::optional<std::chrono::milliseconds> timeout = std::nullopt) noexcept;
void atomic_notify_one(std::atomic<uint32_t> &atomic) noexcept;
void atomic_notify_all(std::atomic<uint32_t> &atomic) noexcept;

namespace detail {
class waiters_guard {
  public:
    waiters_guard(std::atomic<uint32_t> &waiters) noexcept : waiters_(waiters) {
        waiters_.fetch_add(1, std::memory_order_acq_rel);
    }

    ~waiters_guard() { waiters_.fetch_sub(1, std::memory_order_acq_rel); }

  private:
    std::atomic<uint32_t> &waiters_;
};
} // namespace detail

class mutex {
  public:
    constexpr mutex() noexcept : held_(0), waiters_(0) {}

    bool try_lock() noexcept {
        uint32_t held = 0;
        return held_.compare_exchange_strong(held, 1, std::memory_order_acq_rel);
    }

    result<void> lock(std::optional<std::chrono::milliseconds> timeout = std::nullopt) noexcept {
        if (try_lock())
            return ok();

        detail::waiters_guard wg(waiters_);
        while (!try_lock()) {
            try_(atomic_wait(held_, 1, timeout));
        }
        return ok();
    }

    void unlock() noexcept {
        held_.store(0, std::memory_order_release);
        if (waiters_.load(std::memory_order_acquire) != 0)
            atomic_notify_one(held_);
    }

  private:
    std::atomic<uint32_t> held_;
    std::atomic<uint32_t> waiters_;
};

class event {
  public:
    constexpr event(bool signal = false) noexcept : signal_(signal), waiters_(0) {}

    bool try_wait() noexcept { return signal_.load(std::memory_order_acquire); }

    result<void> wait(std::optional<std::chrono::milliseconds> timeout = std::nullopt) noexcept {
        if (try_wait())
            return ok();

        detail::waiters_guard wg(waiters_);
        try_(atomic_wait(signal_, 0, timeout));
        return ok();
    }

    void reset() noexcept { signal_.store(0, std::memory_order_release); }

    void notify_one() noexcept {
        signal_.store(1, std::memory_order_release);
        if (waiters_.load(std::memory_order_acquire) != 0)
            atomic_notify_one(signal_);
    }

    void notify_all() noexcept {
        signal_.store(1, std::memory_order_release);
        if (waiters_.load(std::memory_order_acquire) != 0)
            atomic_notify_all(signal_);
    }

  private:
    std::atomic<uint32_t> signal_;
    std::atomic<uint32_t> waiters_;
};

class auto_reset_event {
  public:
    constexpr auto_reset_event(bool signal = false) noexcept : signal_(signal), waiters_(0) {}

    bool try_wait() noexcept {
        uint32_t signal = 1;
        return signal_.compare_exchange_strong(signal, 0, std::memory_order_acq_rel);
    }

    result<void> wait(std::optional<std::chrono::milliseconds> timeout = std::nullopt) noexcept {
        if (try_wait())
            return ok();

        detail::waiters_guard wg(waiters_);
        while (!try_wait()) {
            try_(atomic_wait(signal_, 0, timeout));
        }
        return ok();
    }

    void notify_one() noexcept {
        signal_.store(1, std::memory_order_release);
        if (waiters_.load(std::memory_order_acquire) != 0)
            atomic_notify_one(signal_);
    }

  private:
    std::atomic<uint32_t> signal_;
    std::atomic<uint32_t> waiters_;
};

class semaphore {
  public:
    constexpr semaphore(uint32_t res = 0) noexcept : res_(res), waiters_(0) {}

    bool try_take() noexcept {
        auto res = res_.load(std::memory_order_acquire);
        while (res) {
            if (res_.compare_exchange_strong(res, res - 1, std::memory_order_acq_rel)) {
                return true;
            }
        }
        return false;
    }

    result<void> take(std::optional<std::chrono::milliseconds> timeout = std::nullopt) noexcept {
        if (try_take())
            return ok();

        detail::waiters_guard wg(waiters_);
        while (!try_take()) {
            try_(atomic_wait(res_, 0, timeout));
        }
        return ok();
    }

    void give() noexcept {
        res_.fetch_add(1, std::memory_order_release);
        if (waiters_.load(std::memory_order_acquire) != 0)
            atomic_notify_one(res_);
    }

  private:
    std::atomic<uint32_t> res_;
    std::atomic<uint32_t> waiters_;
};

// TODO:
class condition_variable;
} // namespace chino::os

namespace std {
template <class T> class unique_lock;

template <> class unique_lock<chino::os::mutex> {
  public:
    CHINO_NONCOPYABLE(unique_lock);

    unique_lock(chino::os::mutex &mutex) noexcept : lock_(mutex) { lock_.lock().expect(nullptr); }
    ~unique_lock() { lock_.unlock(); }

  private:
    chino::os::mutex &lock_;
};
} // namespace std
