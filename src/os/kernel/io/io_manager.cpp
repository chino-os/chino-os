// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "io_manager.h"
#include <chino/conf/board_init.inl>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::kernel::io;

void io::initialize_phase1(const boot_options &options) noexcept { hal::hal_attach_devices(); }
