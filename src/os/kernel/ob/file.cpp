// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include <chino/os/file.h>
#include <chino/os/kernel/io.h>

using namespace chino;
using namespace chino::os;

file ::~file() {
    auto dev = object_.as<device>();
    if (dev.is_ok()) {
        (void)dev.unwrap()->close(*this);
    }
}
