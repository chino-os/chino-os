// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "../handle.h"
#include "../object.h"

namespace chino::os::kernel::ob {
inline constexpr char directory_separator = '/';

class named_object : public object {
  public:
    virtual std::string_view name() const noexcept { return {}; }

    intrusive_list_node directory_list_node;
};

result<void> insert_object(named_object &object, std::string_view full_path) noexcept;
result<object_ptr<named_object>> lookup_object(std::string_view fullpath) noexcept;

template <class T> result<object_ptr<T>> lookup_object(std::string_view fullpath) noexcept {
    try_var(object, lookup_object(fullpath));
    return object.as<T>();
}

result<std::pair<object_ptr<named_object>, std::string_view>>
lookup_object_partial(std::string_view remaining_path) noexcept;

template <class T>
result<std::pair<object_ptr<T>, std::string_view>> lookup_object_partial(std::string_view remaining_path) noexcept {
    try_var(result, lookup_object_partial(remaining_path));
    try_var(object, result.first.as<T>());
    return ok(std::make_pair(std::move(object), result.second));
}

result<std::pair<handle_entry *, int>> alloc_handle(object &object, access_mask granted_access) noexcept;
result<handle_entry *> reference_handle(int handle) noexcept;
result<void> close_handle(int handle) noexcept;

template <class T> result<object_ptr<T>> reference_object(int handle) noexcept {
    try_var(entry, reference_handle(handle));
    auto &obj = entry->object();
    if (obj.is_a(T::kind())) {
        return ok(static_cast<T *>(&obj));
    }
    return err(error_code::bad_cast);
}
} // namespace chino::os::kernel::ob
