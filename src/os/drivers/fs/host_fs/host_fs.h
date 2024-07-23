// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <Windows.h>
#include <chino/conf/board_desc.h>
#include <chino/os/kernel/io.h>

namespace chino::os::drivers {
class host_fs_device : public device {
  public:
    constexpr host_fs_device(hal::meta::board_desc::chip::machine::devices::host_fs) noexcept {}

    std::string_view name() const noexcept override {
        using namespace std::string_view_literals;
        return "fs0"sv;
    }

    result<void> install() noexcept;

    result<file> open(std::string_view path, create_disposition create_disposition) noexcept override;
    result<void> close(file &file) noexcept override;

    result<size_t> read(file &file, std::span<const iovec> iovs, std::optional<size_t> offset) noexcept override;
    result<size_t> write(file &file, std::span<const iovec> iovs, std::optional<size_t> offset) noexcept override;
    result<int> control(file &file, int request, va_list ap) noexcept override;

  private:
    char base_dirname_[MAX_PATH] = {};
};

class host_fs_driver {
  public:
    template <class TDevice, class TBottomDevice> using device_t = host_fs_device;

    static result<void> install_device(host_fs_device &device) noexcept;
};
} // namespace chino::os::drivers
