//
// Chino API
//
#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* Common */

typedef uintptr_t HANDLE;

/* Thread */

#define MAX_THREAD_PRIORITY 5u
#define DEFAULT_THREAD_STACK_SIZE 4 * 1024	// 4 KB

typedef void(*ThreadMain_t)(uintptr_t);

#ifdef __cplusplus
}
#endif
