// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <chino/os/file.h>
#include <chino/result.h>
#include <span>

namespace chino::os::kernel::ps {
class pe_loader {
  public:
    pe_loader() noexcept : image_(nullptr) {}

    result<void> load(std::string_view filepath);
    void *entry() const noexcept;

  private:
    std::byte *image_;
};
} // namespace chino::os::kernel::ps
