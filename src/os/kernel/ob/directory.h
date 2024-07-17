// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <chino/os/kernel/ps.h>
#include <chino/os/object.h>

namespace chino::os::kernel::ob {
class directory : public object {
    CHINO_DEFINE_KERNEL_OBJECT_KIND(object, object_kind_directory);

  public:
    result<void> remove(object &object) noexcept;

    result<void> insert(object &object, std::string_view fullpath) noexcept;
    result<object_ptr<object>> lookup(std::string_view fullpath) noexcept;
    result<std::pair<object_ptr<object>, std::string_view>> lookup_partial(std::string_view remaining_path) noexcept;

  private:
    result<std::pair<object_ptr<object>, std::string_view>> insert_or_lookup(object *insert_object,
                                                                             std::string_view remaining_path) noexcept;

  private:
    intrusive_list<object, &object::directory_list_node> items_;
    ps::mutex items_mutex_;
};
} // namespace chino::os::kernel::ob
