#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "rasterizer.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define NElevation 40
#define NAzimuth   40

#define VertexCount (NElevation * NAzimuth)
#define IndexCount  ((NElevation) * (NAzimuth) * 6)

#define Pi 3.14159265358979323846

#define CameraMouseSpeed 0.1 /* rad/sec */
#define CameraMoveSpeed  10.0 /* units per second, for each dimension */

typedef struct camera {
  vector3 eye;
  float elevation, azimuth;
  float fov;
  float aspect;

  vector3 forward, right, up;
} camera;

void camera_init(camera *camera, float aspect_ratio);

void camera_reorient(camera *camera, float dazimuth, float delevation);
void camera_move(camera *camera, float forward, float right, float up);

mat4 camera_view(const camera *camera);
mat4 camera_projection(const camera *camera);

#define GLSL(code) "#version 330\n"#code

const char *src_vs = GLSL(
  in vec2 pos;
  in vec2 tex;

  out vec2 frag_tex;

  void main() {
    gl_Position = vec4(pos, 0, 1);
    frag_tex = tex;
  }
);

const char *src_fs = GLSL(
  in vec2 frag_tex;
  uniform sampler2D image;

  out vec4 frag_color;

  void main() {
    frag_color = texture(image, frag_tex);
  }
);

const float gl_vertices[] = {
  -1, -1, 0, 0,
  -1, +1, 0, 1,
  +1, -1, 1, 0,
  +1, +1, 1, 1,
};

GLuint create_shader(GLenum mode, const char *src) {
  GLuint ret = glCreateShader(mode);
  GLint length = strlen(src);
  glShaderSource(ret, 1, &src, &length);
  glCompileShader(ret);

  int status;
  char error[256];
  glGetShaderiv(ret, GL_COMPILE_STATUS, &status);
  glGetShaderInfoLog(ret, sizeof(error), 0, error);
  if (!status)
    fprintf(stderr, "compilation error: %s\n", error);

  return ret;
}

void check_link_errors(GLuint prog) {
  int status;
  char error[256];
  glGetProgramiv(prog, GL_LINK_STATUS, &status);
  glGetProgramInfoLog(prog, sizeof(error), 0, error);
  if (!status)
    fprintf(stderr, "Linking error: %s\n", error);
}

int main(int argc, char **argv) {
  glfwInit();

  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);

  GLFWwindow *window = glfwCreateWindow(640, 480, "rasterizer", NULL, NULL);
  glfwMakeContextCurrent(window);
  glewInit();

  vertex *sphere_vertices = malloc(sizeof(vertex) * VertexCount);
  uint32_t *sphere_indices = malloc(sizeof(uint32_t) * IndexCount);

  size_t vertex_i = 0;
  for (size_t i = 0; i < NElevation; i++) {
    for (size_t j = 0; j < NAzimuth; j++) {
      float x = cosf(2*Pi * j / (NAzimuth-1)) * sinf(Pi * i / (NElevation-1));
      float y = sinf(Pi * i / (NElevation-1) - Pi/2);
      float z = sinf(2*Pi * j / (NAzimuth-1)) * sinf(Pi * i / (NElevation-1));

      vertex v = {
        {x, y, z},
        {x, y, z},
        {30, 0, 128, 255},
        {0, 0},
      };

      sphere_vertices[vertex_i++] = v;
    }
  }

  size_t index_i = 0;
  for (size_t i = 0; i < NElevation; i++) {
    for (size_t j = 0; j < NAzimuth; j++) {
      size_t next_j = (j+1) % NAzimuth;
      size_t next_i = (i+1);
      if (i == NElevation-1) next_i = NElevation - 2;

      sphere_indices[index_i++] = j + i*NAzimuth;
      sphere_indices[index_i++] = next_j + i*NAzimuth;
      sphere_indices[index_i++] = j + next_i*NAzimuth;

      sphere_indices[index_i++] = j + next_i*NAzimuth;
      sphere_indices[index_i++] = next_j + i*NAzimuth;
      sphere_indices[index_i++] = next_j + next_i*NAzimuth;
    }
  }

  glEnable(GL_FRAMEBUFFER_SRGB);

  color color_data[640*480];

  mat4 sphere_model = mat4_mul(mat4_translate((vector3){0, 0, 0}),
                               mat4_scale((vector3){3, 3, 3}));

  GLuint prog, fs, vs, vao, vbo, tex;
  prog = glCreateProgram();
  vs = create_shader(GL_VERTEX_SHADER, src_vs);
  fs = create_shader(GL_FRAGMENT_SHADER, src_fs);
  glAttachShader(prog, vs);
  glAttachShader(prog, fs);
  glBindFragDataLocation(prog, 0, "image");
  glBindAttribLocation(prog, 0, "pos");
  glBindAttribLocation(prog, 1, "tex");
  glLinkProgram(prog);
  check_link_errors(prog);

  glUseProgram(prog);
  glUniform1i(glGetUniformLocation(prog, "image"), 0);

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vao);
  glBufferData(GL_ARRAY_BUFFER, sizeof(gl_vertices), gl_vertices,
               GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*4, NULL);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*4,
                        (void*)(2*sizeof(GLfloat)));

  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 640, 480,
               0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  camera camera;
  camera_init(&camera, 640.0/480.0);

  framebuffer fb;
  vertex_array array;
  index_array indices;
  renderer state;

  make_framebuffer(&fb, 640, 480);
  make_vertex_array(&array, VertexCount, sphere_vertices);
  make_index_array(&indices, IndexCount, sphere_indices);
  make_renderer(&state, &fb);
  set_depth_test(&state, true);
  set_culling(&state, true);
  light l = (light){
    {10, 10, 10},
    {25, 25, 25, 255},
    {50, 50, 50, 255},
    {0, 150, 0, 255},
  };
  use_material(
    &state,
    (material){{255,255,255,255},
               {255,255,255,255},
               {255,255,255,255},
                30});
  set_lighting(&state, true);
  set_lights(&state, 1, &l);

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
  glfwSetCursorPos(window, 320, 240);

  float old_time = glfwGetTime();
  while (!glfwWindowShouldClose(window)) {
    float new_time = glfwGetTime();
    float delta_t = (new_time - old_time);
    old_time = new_time;

    set_mvp(&state,
            sphere_model,
            camera_view(&camera),
            camera_projection(&camera));
    clear_color_buffer(&fb, (color){0, 0, 3, 1});
    clear_depth_buffer(&fb, 1);
    draw_elements(&state, DrawTriangles, &indices, &array, 0,
                  IndexCount);
    framebuffer_read(&fb, 0, 0, 640, 480, ColorRGBA, ColorTypeByte,
                     color_data);
    glTexSubImage2D(GL_TEXTURE_2D, 0,
                    0, 0, 640, 480,
                    GL_RGBA, GL_UNSIGNED_BYTE, color_data);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    double mouse_x, mouse_y;
    glfwGetCursorPos(window, &mouse_x, &mouse_y);
    glfwSetCursorPos(window, 320, 240);
    camera_reorient(&camera,
                    CameraMouseSpeed*delta_t*(320 - mouse_x),
                    CameraMouseSpeed*delta_t*(240 - mouse_y));

    float distance = CameraMoveSpeed * delta_t;
    float forward = 0, right = 0, up = 0;

    if (glfwGetKey(window, GLFW_KEY_UP)    == GLFW_PRESS) forward += distance;
    if (glfwGetKey(window, GLFW_KEY_DOWN)  == GLFW_PRESS) forward -= distance;

    if (glfwGetKey(window, GLFW_KEY_LEFT)  == GLFW_PRESS) right -= distance;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) right += distance;

    if (glfwGetKey(window, GLFW_KEY_SPACE)      == GLFW_PRESS) up += distance;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) up -= distance;

    camera_move(&camera, forward, right, up);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  index_array_release(&indices);
  vertex_array_release(&array);
  framebuffer_release(&fb);
  free(sphere_indices);
  free(sphere_vertices);

  glDeleteTextures(1, &tex);
  glDeleteBuffers(1, &vbo);
  glDeleteVertexArrays(1, &vao);
  glDeleteShader(vs);
  glDeleteShader(fs);
  glDeleteProgram(prog);

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}

static float fmod_divsign(float x, float y);

void camera_init(camera *camera, float aspect_ratio) {
  camera->aspect = aspect_ratio;

  camera->eye = (vector3){5, 5, 5};

  camera->elevation = 0;
  camera->azimuth   = Pi/2;

  camera->fov = Pi/4;
  camera_reorient(camera, 0, 0);
}

void camera_reorient(camera *camera, float dazimuth, float delevation) {
  camera->elevation = camera->elevation + delevation;
  camera->azimuth   = camera->azimuth + dazimuth;

  if (camera->elevation > Pi/2) camera->elevation = Pi/2;
  else if (camera->elevation < -Pi/2) camera->elevation = -Pi/2;

  camera->azimuth = fmod_divsign(camera->azimuth, 2*Pi);

  camera->forward = (vector3){
    cosf(camera->elevation)*sinf(camera->azimuth),
    sinf(camera->elevation),
    cosf(camera->elevation)*cosf(camera->azimuth)
  };

  camera->right = (vector3){
    sinf(camera->azimuth - Pi/2),
    0,
    cosf(camera->azimuth - Pi/2)
  };

  camera->up = vector3_cross(camera->right, camera->forward);
}

void camera_move(camera *camera, float forward, float right, float up) {
  camera->eye = vector3_add(camera->eye,
                            vector3_scale(forward, camera->forward));
  camera->eye = vector3_add(camera->eye, vector3_scale(right, camera->right));
  camera->eye = vector3_add(camera->eye, vector3_scale(up, camera->up));
}

mat4 camera_view(const camera *camera) {
  return mat4_look_at(camera->eye,
                      vector3_add(camera->eye, camera->forward),
                      camera->up);
}

mat4 camera_projection(const camera *camera) {
  return mat4_perspective(camera->fov, camera->aspect, 0.1, 2000);
}

static float fmod_divsign(float x, float y) {
  return x - floorf(x/y)*y;
}
