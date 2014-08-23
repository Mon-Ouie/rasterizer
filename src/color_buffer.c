#include "rasterizer.h"

static
void color_buffer_write_float(color *cbuffer, size_t buf_w, size_t buf_h,
                              size_t x, size_t y, size_t w, size_t h,
                              color_format format, const float *buffer);

static
void color_buffer_write_byte(color *cbuffer, size_t buf_w, size_t buf_h,
                             size_t x, size_t y, size_t w, size_t h,
                             color_format format, const uint8_t *buffer);

static
void color_buffer_read_float(const color *cbuffer, size_t buf_w, size_t buf_h,
                             size_t x, size_t y, size_t w, size_t h,
                             color_format format, float *buffer);

static
void color_buffer_read_byte(const color *cbuffer, size_t buf_w, size_t buf_h,
                            size_t x, size_t y, size_t w, size_t h,
                            color_format format, uint8_t *buffer);


void color_buffer_write(color *cbuffer, size_t buf_w, size_t buf_h,
                        size_t x, size_t y, size_t w, size_t h,
                        color_format format, color_type type,
                        const void *buffer) {
  switch (type) {
  case ColorTypeFloat: color_buffer_write_float(cbuffer, buf_w, buf_h,
                                                x, y, w, h, format, buffer);
    break;
  case ColorTypeByte: color_buffer_write_byte(cbuffer, buf_w, buf_h,
                                              x, y, w, h, format, buffer);
    break;
  }
}

void color_buffer_read(const color *cbuffer, size_t buf_w, size_t buf_h,
                       size_t x, size_t y, size_t w, size_t h,
                       color_format format, color_type type, void *buffer) {
  switch (type) {
  case ColorTypeFloat: color_buffer_read_float(cbuffer, buf_w, buf_h,
                                               x, y, w, h, format, buffer);
    break;
  case ColorTypeByte: color_buffer_read_byte(cbuffer, buf_w, buf_h,
                                             x, y, w, h, format, buffer);
    break;
  }
}

static
void color_buffer_write_float(color *cbuffer, size_t buf_w, size_t buf_h,
                              size_t x, size_t y, size_t w, size_t h,
                              color_format format, const float *buffer) {
  for (size_t j = 0; j < h; j++) {
    for (size_t i = 0; i < w; i++) {
      color *pixel = &cbuffer[x+i+(y+j)*buf_w];
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
void color_buffer_write_byte(color *cbuffer, size_t buf_w, size_t buf_h,
                             size_t x, size_t y, size_t w, size_t h,
                             color_format format, const uint8_t *buffer) {
  for (size_t j = 0; j < h; j++) {
    for (size_t i = 0; i < w; i++) {
      color *pixel = &cbuffer[x+i+(y+j)*buf_w];
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
void color_buffer_read_float(const color *cbuffer, size_t buf_w, size_t buf_h,
                             size_t x, size_t y, size_t w, size_t h,
                             color_format format, float *buffer) {
  for (size_t j = 0; j < h; j++) {
    for (size_t i = 0; i < w; i++) {
      const color *pixel = &cbuffer[x+i+(y+j)*buf_w];
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
void color_buffer_read_byte(const color *cbuffer, size_t buf_w, size_t buf_h,
                            size_t x, size_t y, size_t w, size_t h,
                            color_format format, uint8_t *buffer) {
  for (size_t j = 0; j < h; j++) {
    for (size_t i = 0; i < w; i++) {
      const color *pixel = &cbuffer[x+i+(y+j)*buf_w];
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
