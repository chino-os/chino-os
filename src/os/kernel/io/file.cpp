// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include <chino/os/kernel/io.h>
#include <chino/os/kernel/io/device.h>
#include <chino/os/kernel/io/file.h>
#include <chino/os/kernel/kd.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::kernel::io;
using namespace chino::os::kernel::ps;

file ::~file() {
    if (!device_.empty()) {
        (void)close_file(*this);
    }
}

void file::prepare_to_open(io::device &device) noexcept {
    kassert(device_.empty());
    device_ = &device;
}

void file::failed_to_open() noexcept { device_ = nullptr; }
