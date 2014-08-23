#include "rasterizer.h"
#include "color_buffer.h"
#include <stdlib.h>

int make_framebuffer(framebuffer *fb, size_t w, size_t h) {
  fb->w = w;
  fb->h = h;

  fb->color_buffer = malloc(sizeof(*fb->color_buffer)*w*h);
  if (!fb->color_buffer) return -1;

  fb->depth_buffer = malloc(sizeof(*fb->depth_buffer)*w*h);
  if (!fb->depth_buffer) {
    free(fb->color_buffer);
    return -1;
  }

  return 0;
}

void framebuffer_release(framebuffer *fb) {
  free(fb->color_buffer);
  free(fb->depth_buffer);
}

void clear_color_buffer(framebuffer *fb, color c) {
  for (size_t i = 0; i < fb->w*fb->h; i++)
    fb->color_buffer[i] = c;
}

void clear_depth_buffer(framebuffer *fb, float z) {
  for (size_t i = 0; i < fb->w*fb->h; i++)
    fb->depth_buffer[i] = z;
}

void framebuffer_read(const framebuffer *fb,
                      size_t x, size_t y, size_t w, size_t h,
                      color_format format, color_type type, void *buffer) {
  color_buffer_read(fb->color_buffer, fb->w, fb->h,
                    x, y, w, h, format, type, buffer);
}

void depthbuffer_read(const framebuffer *fb,
                      size_t x, size_t y, size_t w, size_t h,
                      float *buffer) {
  for (size_t j = 0; j < h; j++) {
    for (size_t i = 0; i < w; i++) {
      buffer[i+j*w] = fb->depth_buffer[(x+i)+(y+j)*fb->w];
    }
  }
}

size_t framebuffer_width(const framebuffer *fb) {
  return fb->w;
}

size_t framebuffer_height(const framebuffer *fb) {
  return fb->h;
}
