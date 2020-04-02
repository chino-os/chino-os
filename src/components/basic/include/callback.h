#ifndef __CALLBACK_H__
#define __CALLBACK_H__

#include <stdbool.h>

typedef struct
error
{
  int error;
};

typedef bool (*callback)(error err, void* data);

#endif // __CALLBACK_H__
