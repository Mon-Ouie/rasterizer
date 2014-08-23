#include "rasterizer.h"
#include "color_buffer.h"
#include <stdlib.h>

int load_texture(texture *tex, size_t w, size_t h,
                 color_format format, color_type type, const void *buffer) {
  color *own_buffer = malloc(sizeof(*own_buffer) * w * h);
  if (!own_buffer) return -1;

  tex->w    = w;
  tex->h    = h;
  tex->data = own_buffer;

  texture_write(tex, 0, 0, w, h, format, type, buffer);
  return 0;
}

void release_texture(texture *tex) {
  free(tex->data);
}

void texture_write(texture *tex, size_t x, size_t y, size_t w, size_t h,
                   color_format format, color_type type, const void *buffer) {
  color_buffer_write(tex->data, tex->w, tex->h,
                     x, y, w, h, format, type, buffer);
}

void texture_read(const texture *tex, size_t x, size_t y, size_t w, size_t h,
                  color_format format, color_type type, void *buffer) {
  color_buffer_read(tex->data, tex->w, tex->h,
                    x, y, w, h, format, type, buffer);
}

size_t texture_width(const texture *tex) {
  return tex->w;
}

size_t texture_height(const texture *tex) {
  return tex->h;
}
