// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "directory.h"
#include <chino/os/kernel/ob.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::kernel::ob;

namespace {
class root_directory : public ob::directory {
  public:
    std::string_view name() const noexcept override { return {}; }
};
} // namespace

static constinit root_directory root_directory_;

result<void> ob::insert_object(named_object &object, std::string_view full_path) noexcept {
    // 1. Should start with separator
    if (full_path.empty() || full_path.front() != directory_separator)
        return err(error_code::invalid_path);
    return root_directory_.insert(object, full_path.substr(1));
}

result<object_ptr<named_object>> ob::lookup_object(std::string_view fullpath) noexcept {
    // 1. Should start with separator
    if (fullpath.empty() || fullpath.front() != directory_separator)
        return err(error_code::invalid_path);
    return root_directory_.lookup(fullpath.substr(1));
}

result<std::pair<object_ptr<named_object>, std::string_view>>
ob::lookup_object_partial(std::string_view remaining_path) noexcept {
    // 1. Should start with separator
    if (remaining_path.empty() || remaining_path.front() != directory_separator)
        return err(error_code::invalid_path);
    remaining_path = remaining_path.substr(1);
    return root_directory_.lookup_partial(remaining_path);
}
