#include "rasterizer.h"
#include <stdlib.h>
#include <math.h>

typedef struct screen_pos {
  int x, y;
} screen_pos;

static int min(int a, int b);
static int max(int a, int b);

static uint8_t clamp(float v);

static int allocate_buffer(renderer *state, size_t n);
static processed_vertex process_vertex(renderer *state, vertex v);

static void emit_triangle(renderer *state,
                          processed_vertex a, processed_vertex b,
                          processed_vertex c);

static bool cull(renderer *state,
                 processed_vertex a,
                 processed_vertex b,
                 processed_vertex c);

static screen_pos ndc_to_screen(renderer *state, vector3 pos);

static processed_vertex interpolate(processed_vertex a, processed_vertex b,
                                    processed_vertex c,
                                    vector3 coord);
static vector3 interpolate_vector3(vector3 a, vector3 b, vector3 c,
                                   float wfactor, vector3 coord);
static vector2 interpolate_vector2(vector2 a, vector2 b, vector2 c,
                                   float wfactor, vector3 coord);
static color interpolate_color(color a, color b, color c,
                               float wfactor, vector3 coord);

static void draw_fragment(renderer *state, processed_vertex v,
                          screen_pos p);
static bool depth_check(renderer *state, processed_vertex v,
                        screen_pos p);

int draw_array(renderer *state, draw_mode mode,
               vertex_array *array, size_t i, size_t n) {
  if (allocate_buffer(state, n) < 0) return -1;

  for (size_t offset = 0; offset < n; offset++)
    state->vertices[i] = process_vertex(state, array->data[offset+i]);

  switch (mode) {
  case DrawTriangles:
    if (n < 3) return 0;

    for (size_t i = 0; i < n-2; i += 3) {
      emit_triangle(state, state->vertices[i], state->vertices[i+1],
                    state->vertices[i+2]);
    }
    break;
  case DrawTriangleStrip:
    if (n < 3) return 0;

    emit_triangle(state, state->vertices[0], state->vertices[1],
                  state->vertices[2]);

    for (size_t i = 3; i < n; i++) {
      emit_triangle(state, state->vertices[i-1], state->vertices[i-2],
                    state->vertices[i]);
    }
    break;
  case DrawTriangleFan: {
    if (n < 3) return 0;

    processed_vertex first = state->vertices[0];
    for (size_t i = 2; i < n; i++)
      emit_triangle(state, first, state->vertices[i-1], state->vertices[i]);

    break;
  }
  }

  return 0;
}

int draw_elements(renderer *state, draw_mode mode,
                  index_array *indices, vertex_array *array,
                  size_t i, size_t n) {
  if (allocate_buffer(state, array->n) < 0) return -1;

  for (size_t i = 0; i < array->n; i++)
    state->vertices[i].done = false;

  for (size_t offset = 0; offset < n; offset++) {
    uint32_t vertex_i = indices->data[offset + i];
    if (!state->vertices[vertex_i].done)
      state->vertices[vertex_i] = process_vertex(state, array->data[vertex_i]);
  }

  switch (mode) {
  case DrawTriangles:
    if (n < 3) return 0;

    for (size_t i = 0; i < n-2; i += 3) {
      emit_triangle(state,
                    state->vertices[indices->data[i]],
                    state->vertices[indices->data[i+1]],
                    state->vertices[indices->data[i+2]]);
    }
    break;
  case DrawTriangleStrip:
    if (n < 3) return 0;

    emit_triangle(state,
                  state->vertices[indices->data[i]],
                  state->vertices[indices->data[i+1]],
                  state->vertices[indices->data[i+2]]);

    for (size_t offset = 3; offset < n; offset++) {
      emit_triangle(state,
                    state->vertices[indices->data[i+offset-1]],
                    state->vertices[indices->data[i+offset-2]],
                    state->vertices[indices->data[i+offset]]);
    }
    break;
  case DrawTriangleFan: {
    if (n < 3) return 0;

    processed_vertex first = state->vertices[indices->data[i]];
    for (size_t offset = 2; offset < n; offset++)
      emit_triangle(state, first,
                    state->vertices[indices->data[i+offset-1]],
                    state->vertices[indices->data[i+offset]]);

    break;
  }
  }

  return 0;
}

static int allocate_buffer(renderer *state, size_t n) {
  if (state->vertex_count >= n) return 0;

  processed_vertex *buffer = malloc(sizeof(*buffer) * n);
  if (!buffer)
    return -1;

  free(state->vertices);
  state->vertices = buffer;
  state->vertex_count = n;

  return 0;
}

static processed_vertex process_vertex(renderer *state, vertex v) {
  vector3 pos_to_eye = mat4_apply(state->model_view, v.pos);

  processed_vertex out;
  out.done = true;

  out.eye = vector3_scale(-1, pos_to_eye);
  out.normal = vector3_normalize(mat3_apply(state->normal_matrix, v.normal));

  out.tex_coord = v.tex_coord;
  out.base_color = v.col;

  vector4 projected = mat4_project(state->projection, pos_to_eye);
  out.frag_pos = (vector3){projected.x/projected.w, projected.y/projected.w,
                           projected.z/projected.w};
  out.w = projected.w;

  return out;
}

static void emit_triangle(renderer *state,
                          processed_vertex a, processed_vertex b,
                          processed_vertex c) {
  if (!cull(state, a, b, c)) {
    screen_pos p0 = ndc_to_screen(state, a.frag_pos);
    screen_pos p1 = ndc_to_screen(state, b.frag_pos);
    screen_pos p2 = ndc_to_screen(state, c.frag_pos);

    if (p0.y > p1.y) {
      screen_pos t;
      t = p0;
      p0 = p1;
      p1 = t;

      processed_vertex t_v;
      t_v = a;
      a = b;
      b = t_v;
    }

    if (p1.y > p2.y) {
      screen_pos t;
      t = p1;
      p1 = p2;
      p2 = t;

      processed_vertex t_v;
      t_v = b;
      b = c;
      c = t_v;
    }

    if (p0.y > p1.y) {
      screen_pos t;
      t = p0;
      p0 = p1;
      p1 = t;

      processed_vertex t_v;
      t_v = a;
      a = b;
      b = t_v;
    }

    if (p0.y == p2.y) return;

    float det = (p0.x - p2.x)*(p1.y - p2.y) - (p0.y - p2.y)*(p1.x - p2.x);

    int y;
    float dxdy_02 = (float)(p0.x - p2.x) / (p0.y - p2.y);

    if (p0.y != p1.y) {
      float dxdy_01 = (float)(p0.x - p1.x) / (p0.y - p1.y);
      for (y = max(0, p0.y); y <= min(p1.y, state->target->h-1); y++) {
        int dy = y - p0.y;

        int x0 = p0.x + dxdy_01 * dy;
        int x1 = p0.x + dxdy_02 * dy;

        if (x0 > x1) {
          int t;
          t = x0;
          x0 = x1;
          x1 = t;
        }

        for (int x = max(0, x0); x <= min(x1, state->target->w-1); x++) {
          float s = ((p1.y - p2.y)*(x - p2.x) + (p2.x - p1.x)*(y - p2.y)) / det;
          float t = ((p2.y - p2.y)*(x - p0.x) + (p0.x - p2.x)*(y - p2.y)) / det;
          float u = 1 - s - t;

          processed_vertex v = interpolate(a, b, c, (vector3){s, t, u});
          draw_fragment(state, v, (screen_pos){x, y});
        }
      }
    }

    if (p1.y != p2.y) {
      float dxdy_12 = (float)(p1.x - p2.x) / (p1.y - p2.y);
      for (y = max(0, p1.y); y <= min(p2.y, state->target->h-1); y++) {
        int x0 = p1.x + dxdy_12 * (y - p1.y);
        int x1 = p0.x + dxdy_02 * (y - p0.y);

        if (x0 > x1) {
          int t;
          t = x0;
          x0 = x1;
          x1 = t;
        }

        for (int x = max(0, x0); x <= min(x1, state->target->w-1); x++) {
          float s = ((p1.y - p2.y)*(x - p2.x) + (p2.x - p1.x)*(y - p2.y)) / det;
          float t = ((p2.y - p2.y)*(x - p0.x) + (p0.x - p2.x)*(y - p2.y)) / det;
          float u = 1 - s - t;

          processed_vertex v = interpolate(a, b, c, (vector3){s, t, u});
          draw_fragment(state, v, (screen_pos){x, y});
        }
      }
    }
  }
}

static bool cull(renderer *state,
                 processed_vertex a,
                 processed_vertex b,
                 processed_vertex c) {
  if (!state->culling) return false;

  float det =
      a.frag_pos.x*b.frag_pos.y - a.frag_pos.y*b.frag_pos.x
    + b.frag_pos.x*c.frag_pos.y - b.frag_pos.y*c.frag_pos.x
    + c.frag_pos.x*a.frag_pos.y - c.frag_pos.y*a.frag_pos.x;

  return det > 0;
}

static screen_pos ndc_to_screen(renderer *state, vector3 pos) {
  return (screen_pos){
    (+pos.x + 1) * state->target->w / 2,
    (+pos.y + 1) * state->target->h / 2
  };
}

static processed_vertex interpolate(processed_vertex a, processed_vertex b,
                                    processed_vertex c,
                                    vector3 coord) {
  processed_vertex out;

  out.frag_pos = interpolate_vector3(a.frag_pos, b.frag_pos, c.frag_pos,
                                     1.0, coord);

  float wfactor = coord.x/a.w + coord.y/b.w + coord.z/c.w;
  coord.x /= a.w;
  coord.y /= b.w;
  coord.z /= c.w;

  out.eye = interpolate_vector3(a.eye, b.eye, c.eye, wfactor, coord);
  out.normal = interpolate_vector3(a.normal, b.normal, c.normal, wfactor,
                                   coord);

  out.tex_coord = interpolate_vector2(a.tex_coord, b.tex_coord, c.tex_coord,
                                      wfactor, coord);
  out.base_color = interpolate_color(a.base_color, b.base_color, c.base_color,
                                     wfactor, coord);

  return out;
}

static vector3 interpolate_vector3(vector3 a, vector3 b, vector3 c,
                                   float wfactor, vector3 coord) {
  return (vector3){
    (coord.x*a.x + coord.y*b.x + coord.z*c.x)/wfactor,
    (coord.x*a.y + coord.y*b.y + coord.z*c.y)/wfactor,
    (coord.x*a.z + coord.y*b.z + coord.z*c.z)/wfactor,
  };
}

static vector2 interpolate_vector2(vector2 a, vector2 b, vector2 c,
                                   float wfactor, vector3 coord) {
  return (vector2){
    (coord.x*a.x + coord.y*b.x + coord.z*c.x)/wfactor,
    (coord.x*a.y + coord.y*b.y + coord.z*c.y)/wfactor,
  };
}

static color interpolate_color(color a, color b, color c,
                               float wfactor, vector3 coord) {
  return (color){
    (coord.x*a.r + coord.y*b.r + coord.z*c.r)/wfactor,
    (coord.x*a.g + coord.y*b.g + coord.z*c.g)/wfactor,
    (coord.x*a.b + coord.y*b.b + coord.z*c.b)/wfactor,
    (coord.x*a.a + coord.y*b.a + coord.z*c.a)/wfactor,
  };
}

static void draw_fragment(renderer *state, processed_vertex v,
                          screen_pos pos) {
  if (depth_check(state, v, pos)) {
    color tex_color = (color){255,255,255,255};
    if (state->tex) {
      vector2 tex_coord = v.tex_coord;

      if (0 <= tex_coord.x && tex_coord.x <= 1 &&
          0 <= tex_coord.y && tex_coord.y <= 1) {
        int x = tex_coord.x * (state->tex->w-1);
        int y = tex_coord.y * (state->tex->h-1);

        tex_color = state->tex->data[x+y*state->tex->w];
      }
    }

    color light = (color){0,0,0,255};

    if (state->lighting) {
      vector3 n = vector3_normalize(v.normal);
      vector3 e = vector3_normalize(v.eye);

      for (size_t i = 0; i < state->light_count; i++) {
        vector3 l = vector3_normalize(
          vector3_add(e, state->processed_lights[i].pos));
        vector3 r = vector3_reflect(vector3_scale(-1, l), n);

        float diffuse = fmaxf(0, -vector3_dot(l, n));
        float specular = powf(fmaxf(vector3_dot(r, e), 0.0),
                              state->mat.specular_power);

        light.r = clamp(
          light.r +
          state->processed_lights[i].ambient.r +
          diffuse * state->processed_lights[i].diffuse.r +
          specular * state->processed_lights[i].specular.r);
        light.g = clamp(
          light.g +
          state->processed_lights[i].ambient.g +
          diffuse * state->processed_lights[i].diffuse.g +
          specular * state->processed_lights[i].specular.g);
        light.b = clamp(
          light.b +
          state->processed_lights[i].ambient.b +
          diffuse * state->processed_lights[i].diffuse.b +
          specular * state->processed_lights[i].specular.b);
      }
    }
    else
      light = (color){255,255,255,255};

    size_t i = pos.x+pos.y*state->target->w;

    color src = {
      (float)v.base_color.r * (float)tex_color.r/255.0 * (float)light.r/255.0,
      (float)v.base_color.g * (float)tex_color.g/255.0 * (float)light.g/255.0,
      (float)v.base_color.b * (float)tex_color.b/255.0 * (float)light.b/255.0,
      v.base_color.a * tex_color.a/255.0,
    };
    state->target->color_buffer[i] = src;
  }
}

static bool depth_check(renderer *state, processed_vertex v,
                        screen_pos pos) {
  if (state->depth_test_flag){
    size_t i = pos.x+pos.y*state->target->w;
    float dst = state->target->depth_buffer[i];
    float src = v.frag_pos.z;

    bool passed;
    switch (state->depth_func) {
    case DepthTestNever:  passed = false; break;
    case DepthTestAlways: passed = true;  break;

    case DepthTestEQ: passed = src == dst; break;
    case DepthTestLT: passed = src <  dst; break;
    case DepthTestLE: passed = src <= dst; break;
    case DepthTestGT: passed = src >  dst; break;
    case DepthTestGE: passed = src >= dst; break;
    }

    if (passed) state->target->depth_buffer[i] = src;
    return passed;
  }
  else
    return true;
}

static int min(int a, int b) {
  return a > b ? b : a;
}

static int max(int a, int b) {
  return a > b ? a : b;
}

static uint8_t clamp(float v) {
  if (v > 255) return 255;
  if (v < 0) return 0;
  else return v;
}
