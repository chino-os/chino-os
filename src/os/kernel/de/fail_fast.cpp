// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include <chino/result.h>

using namespace chino;

void chino::fail_fast(const char *message) noexcept {
    (void)message;
    while (1)
        ;
}
