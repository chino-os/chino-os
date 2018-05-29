//
// Kernel Debug
//
#pragma once
#include "diagnostic/KernelLogger.hpp"

#define kassert(expression) { if (!(expression)) g_Logger->FailFast(#expression, __FILE__, __LINE__); }
