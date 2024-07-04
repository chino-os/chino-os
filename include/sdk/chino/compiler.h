// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once

#define CHINO_STRINGFY_(x) #x
#define CHINO_STRINGFY(x) CHINO_STRINGFY_(x)

// clang-format off
#define CHINO_CONCAT_2(a, b) a/b
#define CHINO_CONCAT_3(a, b, c) CHINO_CONCAT_2(CHINO_CONCAT_2(a, b), c)
// clang-format on

#define CHINO_MAKE_RELATIVE_HEADER(prefix, target, name) CHINO_STRINGFY(CHINO_CONCAT_3(prefix, target, name))

#ifdef _MSC_VER
#define CHINO_ASSUME(...)
#define CHINO_UNREACHABLE() __assume(0)
#else
#define CHINO_ASSUME(...) __builtin_assume(__VA_ARGS__)
#define CHINO_UNREACHABLE() __builtin_unreachable()
#endif

#define CHINO_NONCOPYABLE(T)                                                                                           \
    T(const T &) = delete;                                                                                             \
    T(T &&) = delete;                                                                                                  \
    T &operator=(const T &) = delete;                                                                                  \
    T &operator=(T &&) = delete
