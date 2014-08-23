#include "rasterizer.h"

#include <stdlib.h>
#include <string.h>

static void update_all_lights(renderer *state);
static void update_light(renderer *state, size_t i);

static color color_mul(color a, color b);

void make_renderer(renderer *state, framebuffer *target) {
  state->target = target;

  state->tex = NULL;

  state->model_view = Mat4Identity;
  state->projection = Mat4Identity;
  state->normal_matrix = Mat3Identity;

  state->mat = (material){
    {255, 255, 255, 255},
    {255, 255, 255, 255},
    {255, 255, 255, 255},
    1
  };

  state->light_count = 0;
  state->lights = NULL;
  state->processed_lights = NULL;

  state->lighting = false;

  state->blend_src = BlendSrcAlpha;
  state->blend_dst = BlendOneMinusSrcAlpha;

  state->depth_func = DepthTestLE;
  state->depth_test_flag = false;

  state->culling = false;
}

void release_renderer(renderer *state) {
  free(state->lights);
  free(state->processed_lights);
}

void use_texture(renderer *state, texture *tex) {
  state->tex = tex;
}

texture *current_texture(const renderer *state) {
  return state->tex;
}

void set_mvp(renderer *state, mat4 model, mat4 view, mat4 projection) {
  state->model_view = mat4_mul(model, view);
  state->projection = projection;

  state->normal_matrix = mat3_transposed_inverse(
    mat4_upper_left_33(state->model_view));

  update_all_lights(state);
}

void use_material(renderer *state, material m) {
  state->mat = m;
  update_all_lights(state);
}

material current_material(const renderer *state) {
  return state->mat;
}

int set_lights(renderer *state, size_t n, light *lights) {
  light *buffer = malloc(n * sizeof(*buffer));
  if (!buffer) return -1;

  light *processed_buffer = malloc(n * sizeof(*processed_buffer));
  if (!processed_buffer) {
    free(buffer);
    return -1;
  }

  free(state->lights);
  free(state->processed_lights);

  state->lights = buffer;
  state->processed_lights = processed_buffer;

  if (lights) {
    memcpy(state->lights, lights, n * sizeof(*lights));
    update_all_lights(state);
  }

  return 0;
}

void set_light(renderer *state, size_t i, light light) {
  state->lights[i] = light;
  update_light(state, i);
}

light get_light(const renderer *state, size_t i) {
  return state->lights[i];
}

void set_lighting(renderer *state, bool on) { state->lighting = on; }
bool get_lighting(const renderer *state) { return state->lighting; }

void set_blend_function(renderer *state,
                        blend_factor sfactor, blend_factor dfactor) {
  state->blend_src = sfactor;
  state->blend_dst = dfactor;
}

void get_blend_function(const renderer *state,
                        blend_factor *sfactor, blend_factor *dfactor) {
  *sfactor = state->blend_src;
  *dfactor = state->blend_dst;
}

void set_depth_func(renderer *state, depth_func f) { state->depth_func = f; }
depth_func get_depth_func(const renderer *state) { return state->depth_func; }

void set_depth_test(renderer *state, bool on) { state->depth_test_flag = on; }
bool get_depth_test(const renderer *state) { return state->depth_test_flag; }

void set_culling(renderer *state, bool on) { state->culling = on; }
bool get_culling(const renderer *state) { return state->culling; }

static void update_all_lights(renderer *state) {
  for (size_t i = 0; i < state->light_count; i++)
    update_light(state, i);
}

static void update_light(renderer *state, size_t i) {
  state->processed_lights[i].pos = mat4_apply(
    state->model_view, state->processed_lights[i].pos);
  state->processed_lights[i].diffuse = color_mul(
    state->mat.diffuse, state->lights[i].ambient);
  state->processed_lights[i].diffuse = color_mul(
    state->mat.diffuse, state->lights[i].diffuse);
  state->processed_lights[i].specular = color_mul(
    state->mat.specular, state->lights[i].specular);
}

static color color_mul(color a, color b) {
  return (color){
    (float)a.r * (float)b.r/255.0,
    (float)a.g * (float)b.g/255.0,
    (float)a.b * (float)b.b/255.0,
    255
  };
}
