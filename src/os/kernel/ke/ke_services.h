// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <chino/os/ke_services.h>

namespace chino::os::kernel {
struct kernel_ke_service_mt : i_ke_services {
  public:
    int errno_() noexcept override;

    int open(const char *pathname, int flags, mode_t mode) noexcept override;
    int close(int fd) noexcept override;

    ssize_t read(int __fd, void *__buf, size_t __nbyte) noexcept override;
    ssize_t write(int __fd, const void *__buf, size_t __nbyte) noexcept override;
    int ioctl(int __fd, int req, void *arg) noexcept override;

    result<void> atomic_wait(std::atomic<uint32_t> &atomic, uint32_t old,
                             std::optional<std::chrono::milliseconds> timeout = std::nullopt) noexcept override;
    void atomic_notify_one(std::atomic<uint32_t> &atomic) noexcept override;
    void atomic_notify_all(std::atomic<uint32_t> &atomic) noexcept override;

    result<async_io_result *> wait_queued_io() noexcept override;

    result<void> read_async(int fd, std::span<std::byte> buffer, size_t offset,
                            async_io_result &result) noexcept override;

    int socket(int domain, int type, int protocol) noexcept override;
    int setsockopt(int sockfd, int level, int option, const void *value, socklen_t value_len) noexcept override;
    ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *to,
                   socklen_t tolen) noexcept override;
    ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *from,
                     socklen_t *fromlen) noexcept override;

    int nanosleep(const struct timespec *rqtp, struct timespec *rmtp) noexcept override;
    int clock_gettime(clockid_t clock_id, struct timespec *tp) noexcept override;
};

int to_errno(error_code code);

template <class T, class Callable> auto wrap_posix(Callable &&callable) noexcept {
    auto r = callable();
    if (r.is_ok()) {
        if constexpr (std::is_void_v<T>) {
            return 0;
        } else {
            return static_cast<T>(r.unwrap());
        }
    } else {
        errno = to_errno(r.unwrap_err());
        return static_cast<std::conditional_t<std::is_void_v<T>, int, T>>(-1);
    }
}

result<void> initialize_ke_services() noexcept;
} // namespace chino::os::kernel
