// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "object.h"

namespace chino::os::kernel::io {
class device : public object {
    CHINO_DEFINE_KERNEL_OBJECT_KIND(object, object_kind_device);
};

class file : public object {
    CHINO_DEFINE_KERNEL_OBJECT_KIND(object, object_kind_file);
};
} // namespace chino::os::kernel::io
