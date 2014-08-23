#include "rasterizer.h"
#include <stdlib.h>
#include <string.h>

int make_vertex_array(vertex_array *array, size_t n, const vertex *data) {
  array->n = n;
  array->data = malloc(sizeof(*array->data) * n);
  if (!array->data) return -1;

  if (data)
    vertex_array_write(array, 0, n, data);

  return 0;
}

void vertex_array_release(vertex_array *array) {
  free(array->data);
}

void vertex_array_write(vertex_array *array, size_t i, size_t n,
                        const vertex *buffer) {
  memcpy(array->data + i, buffer, sizeof(vertex) * n);
}

void vertex_array_read(const vertex_array *array, size_t i, size_t n,
                       vertex *buffer) {
  memcpy(buffer, array->data + i, sizeof(vertex) * n);
}

size_t vertex_array_size(const vertex_array *array) {
  return array->n;
}
