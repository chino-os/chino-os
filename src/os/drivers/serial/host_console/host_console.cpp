// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "host_console.h"

using namespace chino;
using namespace chino::os;
using namespace chino::os::drivers;

namespace {
class host_console_file : public file {
  public:
    result<size_t> read() noexcept override {}
    result<size_t> write() noexcept override {}
    result<void> control() noexcept override {}

    result<void> close() noexcept override {}
};

constinit host_console_file file_;
} // namespace

result<object_ptr<file>> host_console_device::open(std::string_view path,
                                                   create_disposition create_disposition) noexcept {
    return err(error_code::invalid_path);
}

result<void> host_console_driver::install_device(host_console_device &device) noexcept { return ok(); }
