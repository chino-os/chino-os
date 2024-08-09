// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "../device.h"
#include <sys/socket.h>

namespace chino::os::kernel::io {
class socket_device : public device {
  public:
    CHINO_DEFINE_KERNEL_OBJECT_KIND(device, object_kind_socket_device);

    virtual result<void> socket(file &file, int domain, int type, int protocol) noexcept = 0;
    virtual result<void> setsockopt(file &file, int level, int option, const void *value,
                                    socklen_t value_len) noexcept = 0;
    virtual result<size_t> sendto(file &file, const void *buf, size_t len, int flags, const struct sockaddr *to,
                                  socklen_t tolen) noexcept = 0;
    virtual result<size_t> recvfrom(file &file, void *buf, size_t len, int flags, struct sockaddr *from,
                                    socklen_t *fromlen) noexcept = 0;
};
} // namespace chino::os::kernel::io
