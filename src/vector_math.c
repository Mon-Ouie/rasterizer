#include "rasterizer.h"

#include <math.h>
#include <string.h>

#define Pi 3.14159265358979323846

vector2 vector2_add(vector2 a, vector2 b) {
  return (vector2){a.x+b.x, a.y+b.y};
}

vector2 vector2_scale(float f, vector2 a) {
  return (vector2){f*a.x, f*a.y};
}

vector3 vector3_normalize(vector3 v) {
  float norm = sqrtf(v.x*v.x+v.y*v.y+v.z*v.z);
  return (vector3){v.x/norm, v.y/norm, v.z/norm};
}

float vector3_dot(vector3 a, vector3 b) {
  return a.x*b.x+a.y*b.y+a.z*b.z;
}

vector3 vector3_cross(vector3 a, vector3 b) {
  return (vector3){
    a.y*b.z - a.z*b.y,
    a.z*b.x - a.x*b.z,
    a.x*b.y - a.y*b.x
  };
}

vector3 vector3_reflect(vector3 ray, vector3 n) {
  return vector3_sub(ray, vector3_scale(2*fmaxf(vector3_dot(ray, n),0), n));
}

vector3 vector3_sub(vector3 a, vector3 b) {
  return (vector3){a.x-b.x, a.y-b.y, a.z-b.z};
}

vector3 vector3_add(vector3 a, vector3 b) {
  return (vector3){a.x+b.x, a.y+b.y, a.z+b.z};
}

vector3 vector3_mul(vector3 a, vector3 b) {
  return (vector3){a.x*b.x, a.y*b.y, a.z*b.z};
}

vector3 vector3_scale(float f, vector3 a) {
  return (vector3){f*a.x, f*a.y, f*a.z};
}

mat3 mat3_transposed_inverse(mat3 m) {
  float det = mat3_at(m, 0, 0) * mat3_at(m, 1, 1) * mat3_at(m, 2, 2)
            + mat3_at(m, 1, 0) * mat3_at(m, 2, 1) * mat3_at(m, 0, 2)
            + mat3_at(m, 2, 0) * mat3_at(m, 0, 1) * mat3_at(m, 1, 2)
            - mat3_at(m, 0, 2) * mat3_at(m, 1, 1) * mat3_at(m, 2, 0)
            - mat3_at(m, 1, 2) * mat3_at(m, 2, 1) * mat3_at(m, 0, 0)
            - mat3_at(m, 2, 2) * mat3_at(m, 0, 1) * mat3_at(m, 1, 0);

  mat3 inv;

  mat3_at(inv, 0, 0) = + mat3_at(m, 1, 1) * mat3_at(m, 2, 2)
                       - mat3_at(m, 1, 2) * mat3_at(m, 2, 1);
  mat3_at(inv, 1, 0) = - mat3_at(m, 0, 1) * mat3_at(m, 2, 2)
                       + mat3_at(m, 0, 2) * mat3_at(m, 2, 1);
  mat3_at(inv, 2, 0) = + mat3_at(m, 0, 1) * mat3_at(m, 1, 2)
                       - mat3_at(m, 0, 2) * mat3_at(m, 1, 1);

  mat3_at(inv, 0, 1) = - mat3_at(m, 1, 0) * mat3_at(m, 2, 2)
                       + mat3_at(m, 1, 2) * mat3_at(m, 2, 0);
  mat3_at(inv, 1, 1) = + mat3_at(m, 0, 0) * mat3_at(m, 2, 2)
                       - mat3_at(m, 0, 2) * mat3_at(m, 2, 0);
  mat3_at(inv, 2, 1) = - mat3_at(m, 0, 0) * mat3_at(m, 1, 2)
                       + mat3_at(m, 0, 2) * mat3_at(m, 1, 0);

  mat3_at(inv, 0, 2) = + mat3_at(m, 1, 0) * mat3_at(m, 2, 1)
                       - mat3_at(m, 1, 1) * mat3_at(m, 2, 0);
  mat3_at(inv, 1, 2) = - mat3_at(m, 0, 0) * mat3_at(m, 2, 1)
                       + mat3_at(m, 0, 1) * mat3_at(m, 2, 0);
  mat3_at(inv, 2, 2) = + mat3_at(m, 0, 0) * mat3_at(m, 1, 1)
                       - mat3_at(m, 0, 1) * mat3_at(m, 1, 0);

  for (size_t i = 0; i < 9; i++) inv.data[i] /= det;

  return inv;
}

vector3 mat3_apply(mat3 m, vector3 v) {
  return (vector3){
    v.x*mat3_at(m, 0, 0) + v.y*mat3_at(m, 1, 0) + v.z*mat3_at(m, 2, 0),
    v.x*mat3_at(m, 0, 1) + v.y*mat3_at(m, 1, 1) + v.z*mat3_at(m, 2, 1),
    v.x*mat3_at(m, 0, 2) + v.y*mat3_at(m, 1, 2) + v.z*mat3_at(m, 2, 2),
  };
}

mat4 mat4_mul(mat4 a, mat4 b) {
  mat4 out;

  for (size_t i = 0; i < 4; i++) {
    for (size_t j = 0; j < 4; j++) {
      mat4_at(out, j, i) = 0;
      for (size_t k = 0; k < 4; k++)
        mat4_at(out, j, i) += mat4_at(a, k, i) * mat4_at(b, j, k);
    }
  }

  return out;
}

mat3 mat4_upper_left_33(mat4 m) {
  mat3 ret;
  memcpy(&mat3_at(ret, 0, 0), &mat4_at(m, 0, 0), 3*sizeof(float));
  memcpy(&mat3_at(ret, 0, 1), &mat4_at(m, 0, 1), 3*sizeof(float));
  memcpy(&mat3_at(ret, 0, 2), &mat4_at(m, 0, 2), 3*sizeof(float));
  return ret;
}

mat4 mat4_translate(vector3 v) {
  return (mat4){
    {1, 0, 0, v.x,
     0, 1, 0, v.y,
     0, 0, 1, v.z,
     0, 0, 0, 1}
  };
}

mat4 mat4_scale(vector3 v) {
  return (mat4){
    {v.x, 0,   0,   0,
     0,   v.y, 0,   0,
     0,   0,   v.z, 0,
     0,   0,   0,   1}
  };
}

mat4 mat4_look_at(vector3 eye, vector3 center, vector3 up) {
  vector3 f = vector3_normalize(vector3_sub(center, eye));
  up = vector3_normalize(up);

  vector3 s = vector3_cross(f, up);
  vector3 u = vector3_cross(s, f);

  return (mat4){
    { s.x,  s.y,  s.z, -vector3_dot(s, eye),
      u.x,  u.y,  u.z, -vector3_dot(u, eye),
     -f.x, -f.y, -f.z,  vector3_dot(f, eye),
        0,    0,    0,  1}
  };
}

mat4 mat4_perspective(float fov, float aspect, float z_near,
                      float z_far) {
  float f = tanf(Pi/2 - fov/2);

  return (mat4){
    {f / aspect, 0, 0, 0,
     0, f, 0, 0,
     0, 0, (z_far+z_near)/(z_near-z_far), 2*z_far*z_near/(z_near-z_far),
     0, 0, -1, 0}
  };
}

vector3 mat4_apply(mat4 m, vector3 v) {
  return (vector3){
    v.x*mat4_at(m, 0, 0) + v.y*mat4_at(m, 1, 0) + v.z*mat4_at(m, 2, 0) +
      mat4_at(m, 3, 0),
    v.x*mat4_at(m, 0, 1) + v.y*mat4_at(m, 1, 1) + v.z*mat4_at(m, 2, 1) +
      mat4_at(m, 3, 1),
    v.x*mat4_at(m, 0, 2) + v.y*mat4_at(m, 1, 2) + v.z*mat4_at(m, 2, 2) +
      mat4_at(m, 3, 2),
  };
}

vector4 mat4_project(mat4 m, vector3 v) {
  return (vector4){
    v.x*mat4_at(m, 0, 0) + v.y*mat4_at(m, 1, 0) + v.z*mat4_at(m, 2, 0) +
      mat4_at(m, 3, 0),
    v.x*mat4_at(m, 0, 1) + v.y*mat4_at(m, 1, 1) + v.z*mat4_at(m, 2, 1) +
      mat4_at(m, 3, 1),
    v.x*mat4_at(m, 0, 2) + v.y*mat4_at(m, 1, 2) + v.z*mat4_at(m, 2, 2) +
      mat4_at(m, 3, 2),
    v.x*mat4_at(m, 0, 3) + v.y*mat4_at(m, 1, 3) + v.z*mat4_at(m, 2, 3) +
      mat4_at(m, 3, 3),
  };
}
