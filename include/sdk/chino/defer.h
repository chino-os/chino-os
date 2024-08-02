// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "compiler.h"

namespace chino {
template <class TCallable> class defer_guard {
  public:
    constexpr defer_guard(TCallable &&callable) noexcept : callable_(callable) {}
    ~defer_guard() { callable_(); }

  private:
    TCallable callable_;
};
} // namespace chino
