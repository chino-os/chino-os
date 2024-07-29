// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "../result.h"
#include <atomic>
#include <chrono>

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
    constexpr event(bool signal) noexcept : signal_(signal ? 1 : 0) {}

    bool try_wait() noexcept {
        uint32_t signal;
        while (avail = avail_.load(std::memory_order_acquire)) {
            if (avail_.compare_exchange_strong(avail, avail - 1, std::memory_order_acq_rel)) {
                return true;
            }
        }
        return false;
    }

    result<void> wait(std::optional<std::chrono::milliseconds> timeout = std::nullopt) noexcept {
        while (!try_enter()) {
            return atomic_wait(held_, 1, timeout);
        }
    }

    void reset() noexcept { signal_.store(0, std::memory_order_release); }

    void notify_all() noexcept {
        held_.store(0, std::memory_order_release);
        atomic_notify_one(held_);
    }

  private:
    std::atomic<uint32_t> signal_;
};
} // namespace chino::os
