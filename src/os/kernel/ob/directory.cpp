// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "directory.h"
#include <chino/os/kernel/ob.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::kernel::ob;

result<void> directory::remove(object &object) noexcept {
    {
        std::unique_lock<decltype(items_mutex_)> lock(items_mutex_);
        items_.remove(&object);
    }
    object.dec_ref();
    return ok();
}

result<void> directory::insert(object &object, std::string_view full_path) noexcept {
    try_(insert_or_lookup(&object, full_path));
    return ok();
}

result<object_ptr<object>> directory::lookup(std::string_view full_path) noexcept {
    try_var(result, insert_or_lookup(nullptr, full_path));
    return result.second.empty() ? ok(std::move(result.first)) : err(error_code::not_found);
}

result<std::pair<object_ptr<object>, std::string_view>>
directory::lookup_partial(std::string_view remaining_path) noexcept {
    return insert_or_lookup(nullptr, remaining_path);
}

result<std::pair<object_ptr<object>, std::string_view>>
directory::insert_or_lookup(object *insert_object, std::string_view remaining_path) noexcept {
    // 1. Empty name
    if (remaining_path.empty()) {
        if (!insert_object)
            return ok(std::make_pair<object_ptr<object>, std::string_view>(this, {}));
        return err(error_code::invalid_path);
    }

    // 2. Should not start with separator
    if (remaining_path.front() == directory_separator)
        return err(error_code::invalid_path);

    // 3. Find component name
    auto separator_pos = remaining_path.find_first_of(directory_separator);
    auto component_name = remaining_path.substr(0, separator_pos);
    remaining_path =
        separator_pos == remaining_path.npos ? std::string_view{} : remaining_path.substr(separator_pos + 1);

    // 4. Empty component name
    if (component_name.empty()) {
        if (remaining_path.empty() && !insert_object)
            return ok(std::make_pair<object_ptr<object>, std::string_view>(this, {}));
        return err(error_code::invalid_path);
    }

    // 5. Find the component object
    object_ptr<object> component;
    {
        std::unique_lock<decltype(items_mutex_)> lock(items_mutex_);
        auto pivot = items_.front();
        auto find_result = error_code::not_found;
        while (pivot) {
            auto cmp = pivot->name().compare(component_name);
            if (cmp == 0) {
                find_result = error_code::key_already_exists;
                break;
            } else if (cmp > 0) {
                break;
            }
            pivot = items_.next(pivot);
        }

        if (remaining_path.empty()) {
            // 5.1. No remaining, lookup or insert
            if (insert_object) {
                // 5.1.1. insert
                if (find_result == error_code::not_found) {
                    insert_object->add_ref();
                    pivot ? items_.insert_before(pivot, insert_object) : items_.push_back(insert_object);
                    return ok(std::make_pair<object_ptr<object>, std::string_view>(insert_object, {}));
                } else {
                    return err(find_result);
                }
            } else {
                // 5.1.1. lookup
                return find_result == error_code::key_already_exists
                           ? ok(std::make_pair<object_ptr<object>, std::string_view>(pivot, {}))
                           : err(find_result);
            }
        } else {
            // 5.2. Save the component object
            if (find_result == error_code::key_already_exists && pivot) {
                component = pivot;
            } else {
                return err(find_result);
            }
        }
    }

    auto sub_directory = component.as<directory>();
    if (sub_directory.is_ok()) {
        // 6.1. Relay to the sub-directory
        return sub_directory.unwrap()->insert_or_lookup(insert_object, remaining_path);
    } else {
        // 6.2. Return the component with remaining path
        return insert_object ? err(error_code::not_supported)
                             : ok(std::make_pair(std::move(component), remaining_path));
    }
}
