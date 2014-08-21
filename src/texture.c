#include "rasterizer.h"
#include <stdlib.h>

/* evalutes tex more than once */
#define texture_at(tex, x, y) (((tex)->data[(x)+(y)*((tex)->w)]))

static
void texture_write_float(texture *tex, size_t x, size_t y, size_t w, size_t h,
                         color_format format, const float *buffer);

static
void texture_write_byte(texture *tex, size_t x, size_t y, size_t w, size_t h,
                        color_format format, const uint8_t *buffer);

static
void texture_read_float(const texture *tex,
                        size_t x, size_t y, size_t w, size_t h,
                        color_format format, float *buffer);

static
void texture_read_byte(const texture *tex,
                       size_t x, size_t y, size_t w, size_t h,
                       color_format format, uint8_t *buffer);


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
  switch (type) {
  case ColorTypeFloat: texture_write_float(tex, x, y, w, h, format, buffer);
    break;
  case ColorTypeByte: texture_write_byte(tex, x, y, w, h, format, buffer);
    break;
  }
}

void texture_read(const texture *tex, size_t x, size_t y, size_t w, size_t h,
                  color_format format, color_type type, void *buffer) {
  switch (type) {
  case ColorTypeFloat: texture_read_float(tex, x, y, w, h, format, buffer);
    break;
  case ColorTypeByte: texture_read_byte(tex, x, y, w, h, format, buffer);
    break;
  }
}

size_t texture_width(const texture *tex) {
  return tex->w;
}

size_t texture_height(const texture *tex) {
  return tex->h;
}

static
void texture_write_float(texture *tex, size_t x, size_t y, size_t w, size_t h,
                         color_format format, const float *buffer) {
  for (size_t j = 0; j < h; j++) {
    for (size_t i = 0; i < w; i++) {
      color *pixel = &texture_at(tex, x+i, y+j);
      const float *data  = &buffer[(i+w*j)*format];

      switch (format) {
      case ColorGray:
        pixel->r = pixel->g = pixel->b = data[0] * 255;
        pixel->a = 255;
        break;
      case ColorRGB:
        pixel->r = data[0] * 255;
        pixel->g = data[1] * 255;
        pixel->b = data[2] * 255;
        pixel->a = 255;
        break;
      case ColorRGBA:
        pixel->r = data[0] * 255;
        pixel->g = data[1] * 255;
        pixel->b = data[2] * 255;
        pixel->a = data[3] * 255;
        break;
      }
    }
  }
}

static
void texture_write_byte(texture *tex, size_t x, size_t y, size_t w, size_t h,
                        color_format format, const uint8_t *buffer) {
  for (size_t j = 0; j < h; j++) {
    for (size_t i = 0; i < w; i++) {
      color *pixel = &texture_at(tex, x+i, y+j);
      const uint8_t *data  = &buffer[(i+w*j)*format];

      switch (format) {
      case ColorGray:
        pixel->r = pixel->g = pixel->b = data[0];
        pixel->a = 255;
        break;
      case ColorRGB:
        pixel->r = data[0];
        pixel->g = data[1];
        pixel->b = data[2];
        pixel->a = 255;
        break;
      case ColorRGBA:
        pixel->r = data[0];
        pixel->g = data[1];
        pixel->b = data[2];
        pixel->a = data[3];
        break;
      }
    }
  }
}

static
void texture_read_float(const texture *tex,
                        size_t x, size_t y, size_t w, size_t h,
                        color_format format, float *buffer) {
  for (size_t j = 0; j < h; j++) {
    for (size_t i = 0; i < w; i++) {
      const color *pixel = &texture_at(tex, x+i, y+j);
      float *data  = &buffer[(i+w*j)*format];

      switch (format) {
      case ColorGray:
        /* Not actually supported. */
        break;
      case ColorRGB:
        data[0] = (float)pixel->r / 255.0;
        data[1] = (float)pixel->g / 255.0;
        data[2] = (float)pixel->b / 255.0;
        break;
      case ColorRGBA:
        data[0] = (float)pixel->r / 255.0;
        data[1] = (float)pixel->g / 255.0;
        data[2] = (float)pixel->b / 255.0;
        data[3] = (float)pixel->a / 255.0;
        break;
      }
    }
  }
}

static
void texture_read_byte(const texture *tex,
                       size_t x, size_t y, size_t w, size_t h,
                       color_format format, uint8_t *buffer) {
  for (size_t j = 0; j < h; j++) {
    for (size_t i = 0; i < w; i++) {
      const color *pixel = &texture_at(tex, x+i, y+j);
      uint8_t *data  = &buffer[(i+w*j)*format];

      switch (format) {
      case ColorGray:
        /* Not actually supported. */
        break;
      case ColorRGB:
        data[0] = pixel->r;
        data[1] = pixel->g;
        data[2] = pixel->b;
        break;
      case ColorRGBA:
        data[0] = pixel->r;
        data[1] = pixel->g;
        data[2] = pixel->b;
        data[3] = pixel->a;
        break;
      }
    }
  }
}
