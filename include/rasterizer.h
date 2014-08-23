#ifndef RASTERIZER_H_
#define RASTERIZER_H_

#include <stdint.h>
#include <stddef.h>

typedef struct color {
  uint8_t r, g, b, a;
} color;

typedef struct texture {
  size_t w, h;
  color *data;
} texture;

typedef enum color_format {
  ColorGray = 1,
  ColorRGB  = 3,
  ColorRGBA = 4,
} color_format;

typedef enum color_type {
  ColorTypeFloat,
  ColorTypeByte,
} color_type;

typedef struct framebuffer {
  size_t w, h;
  color *color_buffer;
  float *depth_buffer;
} framebuffer;

/* Textures */

int load_texture(texture *tex, size_t w, size_t h,
                 color_format format, color_type type, const void *buffer);
void release_texture(texture *tex);

void texture_write(texture *tex, size_t x, size_t y, size_t w, size_t h,
                   color_format format, color_type type, const void *buffer);
void texture_read(const texture *tex, size_t x, size_t y, size_t w, size_t h,
                  color_format format, color_type type, void *buffer);

size_t texture_width(const texture *tex);
size_t texture_height(const texture *tex);

/* Framebuffer manipulation */

int make_framebuffer(framebuffer *fb, size_t w, size_t h);
void framebuffer_release(framebuffer *fb);

void clear_color_buffer(framebuffer *fb, color c);
void clear_depth_buffer(framebuffer *fb, float z);

void framebuffer_read(const framebuffer *fb,
                      size_t x, size_t y, size_t w, size_t h,
                      color_format format, color_type type, void *buffer);
void depthbuffer_read(const framebuffer *fb,
                      size_t x, size_t y, size_t w, size_t h,
                      float *buffer);

size_t framebuffer_width(const framebuffer *fb);
size_t framebuffer_height(const framebuffer *fb);

#endif
