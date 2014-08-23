#include <stdio.h>
#include <stdbool.h>
#include "rasterizer.h"

int main(int argc, char **argv) {
  uint8_t data[100*50*3];
  for (size_t j = 0; j < 100; j++) {
    for (size_t i = 0; i < 50; i++) {
      data[(i+50*j)*3 + 0] = i;
      data[(i+50*j)*3 + 1] = j;
      data[(i+50*j)*3 + 2] = 42;
    }
  }

  texture tex;

  load_texture(&tex, 50, 100, ColorRGB, ColorTypeByte, data); {
    printf("texture size: %zux%zu\n",
           texture_width(&tex), texture_height(&tex));

    color out_data[20*30];
    texture_read(&tex, 10, 20, 30, 20, ColorRGBA, ColorTypeByte, out_data);
    printf("RGBA at (13, 25): (%d, %d, %d, %d)\n",
           out_data[(3 + 5*30)].r,
           out_data[(3 + 5*30)].g,
           out_data[(3 + 5*30)].b,
           out_data[(3 + 5*30)].a);

    float column[100];
    for (size_t i = 0; i < 100; i++) column[i] = (float)i/100.0;
    texture_write(&tex, 15, 0, 1, 100, ColorGray, ColorTypeFloat, column);

    float out_column[100*3];
    texture_read(&tex, 15, 0, 1, 100, ColorRGB, ColorTypeFloat, out_column);
    printf("RGB in the middle: (%f, %f, %f)\n",
           out_column[50*3+0], out_column[50*3+1], out_column[50*3+2]);
  } release_texture(&tex);

  framebuffer fb;

  make_framebuffer(&fb, 10, 15); {
    printf("frame buffer size: %zux%zu\n",
           framebuffer_width(&fb), framebuffer_height(&fb));

    clear_color_buffer(&fb, (color){10,20,30,40});
    clear_depth_buffer(&fb, 0.25);

    color colors[25];
    float depth[25];

    framebuffer_read(&fb, 3, 4, 5, 5, ColorRGBA, ColorTypeByte, colors);
    depthbuffer_read(&fb, 0, 2, 5, 5, depth);

    bool all_right = true;
    for (size_t i = 0; i < 25; i++) {
      if (colors[i].r != 10 || colors[i].g != 20 || colors[i].b != 30 ||
          colors[i].a != 40) all_right = false;
    }
    printf("Color check: %s\n", all_right ? "ok" : "ko");

    all_right = true;
    for (size_t i = 0; i < 25; i++) {
      if (depth[i] != 0.25) all_right = false;
    }
    printf("Depth check: %s\n", all_right ? "ok" : "ko");
  } framebuffer_release(&fb);

  return 0;
}
