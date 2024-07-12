// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "../object.h"
#include <chino/buffer_slist.h>

namespace chino::os {
class device;

enum class std_handles { in, out, err };

enum class create_disposition {
    create_always = 2,
    create_new = 1,
    open_always = 4,
    open_existing = 3,
    truncate_existing = 5
};

using control_code_t = uint32_t;

class file : public object {
    CHINO_DEFINE_KERNEL_OBJECT_KIND(object, object_kind_file);

  public:
    virtual result<size_t> read(const buffer_slist &buffer) noexcept = 0;
    virtual result<size_t> write(const buffer_slist &buffer) noexcept = 0;
    virtual result<size_t> control(control_code_t code, const buffer_slist &in_buffer,
                                   const buffer_slist &out_buffer) noexcept = 0;

    virtual result<void> close() noexcept = 0;
};

class device : public object {
    CHINO_DEFINE_KERNEL_OBJECT_KIND(object, object_kind_device);

  public:
    virtual result<object_ptr<file>> open(std::string_view path, create_disposition create_disposition) noexcept = 0;
};
} // namespace chino::os
