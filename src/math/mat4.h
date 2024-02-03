#ifndef MAT4_H
#define MAT4_H

#include "vec3.h"

typedef struct quat
{
    float x, y, z, w;
} quat;

typedef struct
{
    float v[4][4];
} mat4;

mat4 mat4_identity();
mat4 mat4_cast(quat q);

mat4 mat4_perspective(float fov_y, float aspect, float near, float far);
mat4 mat4_ortho(float left, float right, float bottom, float top, float near, float far);

mat4 mat4_look_at(vec3 eye, vec3 look_at, vec3 up);

mat4 mat4_rotation(vec3 axis, float angle); /* angle in radians */
mat4 mat4_translation(vec3 v);
mat4 mat4_scale(vec3 v);

mat4 mat4_rotate_x(mat4 mat, float f);
mat4 mat4_rotate_y(mat4 mat, float f);
mat4 mat4_rotate_z(mat4 mat, float f);

mat4 mat4_multiply(mat4 l, mat4 r);

mat4 mat4_invert(mat4 m);

mat4 mat4_interpolate(mat4 mat0, mat4 mat1, float time);

quat quat_identity();

quat quat_slerp(quat q0, quat q1, float value);
quat quat_cast(mat4 mat);

#endif /* !MAT4_H */
