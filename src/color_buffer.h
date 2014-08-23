#ifndef COLOR_BUFFER_H_
#define COLOR_BUFFER_H_

#include "rasterizer.h"

void color_buffer_write(color *cbuffer, size_t buf_w, size_t buf_h,
                        size_t x, size_t y, size_t w, size_t h,
                        color_format format, color_type type,
                        const void *buffer);

void color_buffer_read(const color *cbuffer, size_t buf_w, size_t buf_h,
                       size_t x, size_t y, size_t w, size_t h,
                       color_format format, color_type type, void *buffer);

#endif
