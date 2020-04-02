#include <stdlib.h>
#include <string.h>

#include <stdio.h>

#include "array.h"

struct array {
  size_t element_size;
  size_t size;
  char* ptr;
};

  array*
array_new(size_t element_size)
{
  // printf("array size: %ld\n", sizeof(array));
  array* a = malloc(sizeof(array));  
  a->element_size = element_size;
  a->size = 0;
  a->ptr = NULL;
  return a;
}

  array*
array_alloc(array* array, size_t size)
{
  array->size = size;
  array->ptr = realloc(array->ptr, array->element_size * array->size);
  return array;
}

  void
array_destroy(array* array)
{
  free(array->ptr);
  free(array);
}

  void*
array_push(array* array, void* value)
{
  array->size++;
  array->ptr = realloc(array->ptr, array->element_size * array->size);
  void* element = array->ptr + array->element_size * (array->size - 1);
  memcpy(element, value, array->element_size);
  return element;
}

  void*
array_get(array* array, size_t index)
{
  return array->ptr + index * array->element_size;
}

  void*
array_set(array* array, size_t index, void* value)
{
  void* element = array_get(array, index);
  memcpy(element, value, array->element_size);
  return element;
}

  size_t
array_size(array* array)
{
  return array->size;
}

