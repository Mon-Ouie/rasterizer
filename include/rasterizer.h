#ifndef RASTERIZER_H_
#define RASTERIZER_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct color {
  uint8_t r, g, b, a;
} color;

typedef struct vector3 {
  float x, y, z;
} vector3;

typedef struct vector2 {
  float x, y;
} vector2;

typedef struct mat4 {
  float data[16];
} mat4;

typedef struct mat3 {
  float data[9];
} mat3;

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

typedef struct vertex {
  vector3 pos;
  vector3 normal;
  color col;
  vector2 tex_coord;
} vertex;

typedef struct vertex_array {
  size_t n;
  vertex *data;
} vertex_array;

typedef struct light {
  vector3 pos;
  color ambient, diffuse, specular;
} light;

typedef struct material {
  color ambient, diffuse, specular;
  float specular_power;
} material;

typedef enum blend_factor {
  BlendZero,
  BlendOne,

  BlendSrcColor,
  BlendDstColor,
  BlendOneMinusDstColor,
  BlendOneMinusSrcColor,

  BlendSrcAlpha,
  BlendDstAlpha,
  BlendOneMinusSrcAlpha,
  BlendOneMinusDstAlpha,
} blend_factor;

typedef enum depth_func {
  DepthTestNever,
  DepthTestAlways,

  DepthTestEQ,
  DepthTestLT,
  DepthTestLE,
  DepthTestGT,
  DepthTestGE,
} depth_func;

typedef struct renderer {
  framebuffer *target;

  texture *tex;

  mat4 model_view;
  mat4 projection;
  mat3 normal_matrix;

  material mat;

  size_t light_count;
  light *lights;
  light *processed_lights;

  bool lighting;

  blend_factor blend_src;
  blend_factor blend_dst;

  depth_func depth_func;
  bool depth_test_flag;

  bool culling;
} renderer;

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

/* Vertex arrays */

int make_vertex_array(vertex_array *array, size_t n, const vertex *data);
void vertex_array_release(vertex_array *array);

void vertex_array_write(vertex_array *array, size_t i, size_t n,
                        const vertex *buffer);
void vertex_array_read(const vertex_array *array, size_t i, size_t n,
                       vertex *buffer);

size_t vertex_array_size(const vertex_array *array);

/* Vector algebra */

vector3 vector3_normalize(vector3 v);

float vector3_dot(vector3 a, vector3 b);
vector3  vector3_cross(vector3 a, vector3 b);

vector3 vector3_sub(vector3 a, vector3 b);
vector3 vector3_add(vector3 a, vector3 b);
vector3 vector3_mul(vector3 a, vector3 b);

vector3 vector3_scale(float f, vector3 a);

#define mat3_at(m, x, y) ((m).data[(x)+3*(y)])

#define Mat3Identity \
  (mat3){            \
    {1, 0, 0,        \
     0, 1, 0,        \
     0, 0, 1}        \
  }


mat3 mat3_transposed_inverse(mat3 m);

#define mat4_at(m, x, y) ((m).data[(x)+4*(y)])

#define Mat4Identity \
  (mat4){            \
    {1, 0, 0, 0,     \
     0, 1, 0, 0,     \
     0, 0, 1, 0,     \
     0, 0, 0, 1}     \
  }

mat4 mat4_mul(mat4 a, mat4 b);

mat3 mat4_upper_left_33(mat4 m);

mat4 mat4_translate(vector3 v);
mat4 mat4_scale(vector3 v);
mat4 mat4_look_at(vector3 eye, vector3 center, vector3 up);
mat4 mat4_perspective(float fov, float aspect, float z_near, float z_far);

vector3 mat4_apply(mat4 m, vector3 v);

/* Renderer state */

void make_renderer(renderer *state, framebuffer *target);
void release_renderer(renderer *state);

void use_texture(renderer *state, texture *tex);
texture *current_texture(const renderer *state);

void set_mvp(renderer *state, mat4 model, mat4 view, mat4 projection);

void use_material(renderer *state, material m);
material current_material(const renderer *state);

int set_lights(renderer *state, size_t n, light *lights);
void set_light(renderer *state, size_t i, light light);
light get_light(const renderer *state, size_t i);

void set_lighting(renderer *state, bool on);
bool get_lighting(const renderer *state);

void set_blend_function(renderer *state,
                        blend_factor sfactor, blend_factor dfactor);
void get_blend_function(const renderer *state,
                        blend_factor *sfactor, blend_factor *dfactor);

void set_depth_func(renderer *state, depth_func f);
depth_func get_depth_func(const renderer *state);

void set_depth_test(renderer *state, bool on);
bool get_depth_test(const renderer *state);

void set_culling(renderer *state, bool on);
bool get_culling(const renderer *state);

#endif
