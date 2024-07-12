// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "compiler.h"

namespace chino {

struct buffer_slist_node {
    std::byte *start;
    uintptr_t size;
    buffer_slist_node *next;
};

struct buffer_slist {
    buffer_slist_node *head;
};
} // namespace chino
