#include "rasterizer.h"
#include <stdlib.h>
#include <string.h>

int make_index_array(index_array *array, size_t n, const uint32_t *data) {
  array->n = n;
  array->data = malloc(sizeof(*array->data) * n);
  if (!array->data) return -1;

  if (data)
    index_array_write(array, 0, n, data);

  return 0;
}

void index_array_release(index_array *array) {
  free(array->data);
}

void index_array_write(index_array *array, size_t i, size_t n,
                        const uint32_t *buffer) {
  memcpy(array->data + i, buffer, sizeof(uint32_t) * n);
}

void index_array_read(const index_array *array, size_t i, size_t n,
                       uint32_t *buffer) {
  memcpy(buffer, array->data + i, sizeof(uint32_t) * n);
}

size_t index_array_size(const index_array *array) {
  return array->n;
}
