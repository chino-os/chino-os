// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "driver.h"

using namespace chino;
using namespace chino::os::drivers;

result<void> host_console_driver::attach_device(host_console_device &device) noexcept { return ok(); }
