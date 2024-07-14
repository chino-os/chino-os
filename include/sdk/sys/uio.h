// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct iovec {
    void *iov_base; /* Base address of I/O memory region */
    size_t iov_len; /* Size of the memory pointed to by iov_base */
};

#ifdef __cplusplus
}
#endif
