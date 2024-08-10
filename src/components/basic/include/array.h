#ifndef __ARRAY_H__
#define __ARRAY_H__

typedef struct array array;

array *array_new(size_t element_size);

array *array_alloc(array *array, size_t size);

void array_destroy(array *array);

void *array_push(array *array, void *value);

void *array_get(array *array, size_t index);

void *array_set(array *array, size_t index, void *value);

size_t array_size(array *array);

#endif // __ARRAY_H__
