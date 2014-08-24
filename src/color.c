#include "rasterizer.h"

static uint8_t clamp(int v);

color color_mul(color a, color b) {
  return (color){
    (float)a.r * (float)b.r/255.0,
    (float)a.g * (float)b.g/255.0,
    (float)a.b * (float)b.b/255.0,
    (float)a.a * (float)b.a/255.0,
  };
}

color color_add(color a, color b) {
  int vr = (int)a.r + b.r;
  int vg = (int)a.b + b.b;
  int vb = (int)a.g + b.g;

  return (color){clamp(vr), clamp(vg), clamp(vb), a.a};
}

color color_scale(float f, color a) {
  return (color){clamp(f*a.r), clamp(f*a.g), clamp(f*a.b), a.a};
}

static uint8_t clamp(int v) {
  if (v > 255) return 255;
  if (v < 0) return 0;
  else return v;
}
