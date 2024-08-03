// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "../device.h"

namespace chino::os::kernel::io {
class stdio_device : public device {
  public:
    CHINO_DEFINE_KERNEL_OBJECT_KIND(device, object_kind_stdio_device);
};
} // namespace chino::os::kernel::io
