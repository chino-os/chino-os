// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include <chino/os/kernel/object.h>

using namespace chino::os::kernel;

bool object::is_a([[maybe_unused]] const object_kind &kind) const noexcept { return false; }
