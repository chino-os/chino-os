//
// Kernel Debug
//
#pragma once
#include "diagnostic/KernelLogger.hpp"

#define kassert(expression) { if (!(expression)) g_Logger->FailFast(__FILE__, __LINE__); }
