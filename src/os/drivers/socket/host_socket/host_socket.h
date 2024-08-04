// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "../../../hal/archs/emulator/emulator.h"
#include <chino/conf/board_desc.h>
#include <chino/os/kernel/io.h>
#include <chino/os/kernel/io/devices/socket_device.h>
#include <chino/os/kernel/ps.h>

namespace chino::os::drivers {
class host_socket_device : public kernel::io::socket_device {
  public:
    constexpr host_socket_device(hal::meta::board_desc::chip::machine::devices::host_socket) noexcept {}

    std::string_view name() const noexcept override {
        using namespace std::string_view_literals;
        return "socket"sv;
    }

    result<void> install() noexcept;

    result<void> close(kernel::io::file &file) noexcept override;
    virtual result<void> socket(kernel::io::file &file, int domain, int type, int protocol) noexcept override;
    virtual result<void> setsockopt(kernel::io::file &file, int level, int option, const void *value,
                                    socklen_t value_len) noexcept override;
    virtual result<size_t> sendto(kernel::io::file &file, const void *buf, size_t len, int flags,
                                  const struct sockaddr *to, socklen_t tolen) noexcept override;
    virtual result<size_t> recvfrom(kernel::io::file &file, void *buf, size_t len, int flags, struct sockaddr *from,
                                    socklen_t *fromlen) noexcept override;
};

class host_socket_driver {
  public:
    template <class TDevice, class TBottomDevice> using device_t = host_socket_device;

    static result<void> install_device(host_socket_device &device) noexcept;
};
} // namespace chino::os::drivers
