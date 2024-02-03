#include "mat4.h"

#include <math.h>

mat4 mat4_identity()
{
    mat4 result = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    return result;
}

mat4 mat4_perspective(float fov_y, float aspect, float near, float far)
{
    float tan_half_fov_y = 1.0f / tanf(fov_y * 0.5f);

    mat4 result = { 0.0f };
    result.v[0][0] = tan_half_fov_y / aspect;
    result.v[1][1] = tan_half_fov_y;
    result.v[2][2] = -((far + near) / (far - near));
    result.v[2][3] = -1.f;
    result.v[3][2] = -((2.f * far * near) / (far - near));

    return result;
}

mat4 mat4_ortho(float left, float right, float bottom, float top, float near, float far)
{
    float rl = 1.0f / (right - left);
    float tb = 1.0f / (top - bottom);
    float fn = -1.0f / (far - near);

    mat4 result = { 0.0f };
    result.v[0][0] = 2.0f * rl;
    result.v[1][1] = 2.0f * tb;
    result.v[2][2] = 2.0f * fn;
    result.v[3][0] = -(left + right) * rl;
    result.v[3][1] = -(top + bottom) * tb;
    result.v[3][2] = (far + near) * fn;
    result.v[3][3] = 1.f;

    return result;
}

mat4 mat4_rotation(vec3 axis, float angle)
{
    float c = cosf(angle);
    float s = sinf(angle);
    float one_c = 1.0f - c;
    float x = axis.x;
    float y = axis.y;
    float z = axis.z;
    float xx = x * x;
    float xy = x * y;
    float xz = x * z;
    float yy = y * y;
    float yz = y * z;
    float zz = z * z;
    float l = xx + yy + zz;
    float sqrt_l = sqrtf(l);

    mat4 result = { 0.0f };
    result.v[0][0] = (xx + (yy + zz) * c) / l;
    result.v[0][1] = (xy * one_c + z * sqrt_l * s) / l;
    result.v[0][2] = (xz * one_c - y * sqrt_l * s) / l;
    result.v[1][0] = (xy * one_c - z * sqrt_l * s) / l;
    result.v[1][1] = (yy + (xx + zz) * c) / l;
    result.v[1][2] = (yz * one_c + x * sqrt_l * s) / l;
    result.v[2][0] = (xz * one_c + y * sqrt_l * s) / l;
    result.v[2][1] = (yz * one_c - x * sqrt_l * s) / l;
    result.v[2][2] = (zz + (xx + yy) * c) / l;
    result.v[3][3] = 1.0f;
    return result;
}

mat4 mat4_translation(vec3 v)
{
    mat4 result = mat4_identity();
    result.v[3][0] = v.x;
    result.v[3][1] = v.y;
    result.v[3][2] = v.z;
    return result;
}