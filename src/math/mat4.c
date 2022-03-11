#include "mat4.h"

#include <math.h> 

float mat4_determinant(mat4 mat)
{
    float m11 = mat.v[0];
    float m21 = mat.v[1];
    float m31 = mat.v[2];
    float m41 = mat.v[3];
    float m12 = mat.v[4];
    float m22 = mat.v[5];
    float m32 = mat.v[6];
    float m42 = mat.v[7];
    float m13 = mat.v[8];
    float m23 = mat.v[9];
    float m33 = mat.v[10];
    float m43 = mat.v[11];
    float m14 = mat.v[12];
    float m24 = mat.v[13];
    float m34 = mat.v[14];
    float m44 = mat.v[15];
    float determinant = m14 * m23 * m32 * m41 - m13 * m24 * m32 * m41
        - m14 * m22 * m33 * m41 + m12 * m24 * m33 * m41
        + m13 * m22 * m34 * m41 - m12 * m23 * m34 * m41
        - m14 * m23 * m31 * m42 + m13 * m24 * m31 * m42
        + m14 * m21 * m33 * m42 - m11 * m24 * m33 * m42
        - m13 * m21 * m34 * m42 + m11 * m23 * m34 * m42
        + m14 * m22 * m31 * m43 - m12 * m24 * m31 * m43
        - m14 * m21 * m32 * m43 + m11 * m24 * m32 * m43
        + m12 * m21 * m34 * m43 - m11 * m22 * m34 * m43
        - m13 * m22 * m31 * m44 + m12 * m23 * m31 * m44
        + m13 * m21 * m32 * m44 - m11 * m23 * m32 * m44
        - m12 * m21 * m33 * m44 + m11 * m22 * m33 * m44;
    return determinant;
}

mat4 mat4_indentity()
{
    mat4 result;
    result.v[0] = 1.0f; result.v[4] = 0.0f; result.v[8] = 0.0f; result.v[12] = 0.0f;
    result.v[1] = 0.0f; result.v[5] = 1.0f; result.v[9] = 0.0f; result.v[13] = 0.0f;
    result.v[2] = 0.0f; result.v[6] = 0.0f; result.v[10] = 1.0f;result.v[14] = 0.0f;
    result.v[3] = 0.0f; result.v[7] = 0.0f; result.v[11] = 0.0f;result.v[15] = 1.0f;

    return result;
}

mat4 mat4_transpose(mat4 mat)
{
    mat4 result;

    result.v[0] = mat.v[0];
    result.v[1] = mat.v[4];
    result.v[2] = mat.v[8];
    result.v[3] = mat.v[12];
    result.v[4] = mat.v[1];
    result.v[5] = mat.v[5];
    result.v[6] = mat.v[9];
    result.v[7] = mat.v[13];
    result.v[8] = mat.v[2];
    result.v[9] = mat.v[6];
    result.v[10] = mat.v[10];
    result.v[11] = mat.v[14];
    result.v[12] = mat.v[3];
    result.v[13] = mat.v[7];
    result.v[14] = mat.v[11];
    result.v[15] = mat.v[15];

    return result;
}

mat4 mat4_inverse(mat4 mat)
{
	mat4 inverse;
	float inverted_determinant;
	float m11 = mat.v[0];
	float m21 = mat.v[1];
	float m31 = mat.v[2];
	float m41 = mat.v[3];
	float m12 = mat.v[4];
	float m22 = mat.v[5];
	float m32 = mat.v[6];
	float m42 = mat.v[7];
	float m13 = mat.v[8];
	float m23 = mat.v[9];
	float m33 = mat.v[10];
	float m43 = mat.v[11];
	float m14 = mat.v[12];
	float m24 = mat.v[13];
	float m34 = mat.v[14];
	float m44 = mat.v[15];
	inverse.v[0] = m22 * m33 * m44 - m22 * m43 * m34 - m23 * m32 * m44 + m23 * m42 * m34 + m24 * m32 * m43 - m24 * m42 * m33;
	inverse.v[4] = -m12 * m33 * m44 + m12 * m43 * m34 + m13 * m32 * m44 - m13 * m42 * m34 - m14 * m32 * m43 + m14 * m42 * m33;
	inverse.v[8] = m12 * m23 * m44 - m12 * m43 * m24 - m13 * m22 * m44 + m13 * m42 * m24 + m14 * m22 * m43 - m14 * m42 * m23;
	inverse.v[12] = -m12 * m23 * m34 + m12 * m33 * m24 + m13 * m22 * m34 - m13 * m32 * m24 - m14 * m22 * m33 + m14 * m32 * m23;
	inverse.v[1] = -m21 * m33 * m44 + m21 * m43 * m34 + m23 * m31 * m44 - m23 * m41 * m34 - m24 * m31 * m43 + m24 * m41 * m33;
	inverse.v[5] = m11 * m33 * m44 - m11 * m43 * m34 - m13 * m31 * m44 + m13 * m41 * m34 + m14 * m31 * m43 - m14 * m41 * m33;
	inverse.v[9] = -m11 * m23 * m44 + m11 * m43 * m24 + m13 * m21 * m44 - m13 * m41 * m24 - m14 * m21 * m43 + m14 * m41 * m23;
	inverse.v[13] = m11 * m23 * m34 - m11 * m33 * m24 - m13 * m21 * m34 + m13 * m31 * m24 + m14 * m21 * m33 - m14 * m31 * m23;
	inverse.v[2] = m21 * m32 * m44 - m21 * m42 * m34 - m22 * m31 * m44 + m22 * m41 * m34 + m24 * m31 * m42 - m24 * m41 * m32;
	inverse.v[6] = -m11 * m32 * m44 + m11 * m42 * m34 + m12 * m31 * m44 - m12 * m41 * m34 - m14 * m31 * m42 + m14 * m41 * m32;
	inverse.v[10] = m11 * m22 * m44 - m11 * m42 * m24 - m12 * m21 * m44 + m12 * m41 * m24 + m14 * m21 * m42 - m14 * m41 * m22;
	inverse.v[14] = -m11 * m22 * m34 + m11 * m32 * m24 + m12 * m21 * m34 - m12 * m31 * m24 - m14 * m21 * m32 + m14 * m31 * m22;
	inverse.v[3] = -m21 * m32 * m43 + m21 * m42 * m33 + m22 * m31 * m43 - m22 * m41 * m33 - m23 * m31 * m42 + m23 * m41 * m32;
	inverse.v[7] = m11 * m32 * m43 - m11 * m42 * m33 - m12 * m31 * m43 + m12 * m41 * m33 + m13 * m31 * m42 - m13 * m41 * m32;
	inverse.v[11] = -m11 * m22 * m43 + m11 * m42 * m23 + m12 * m21 * m43 - m12 * m41 * m23 - m13 * m21 * m42 + m13 * m41 * m22;
	inverse.v[15] = m11 * m22 * m33 - m11 * m32 * m23 - m12 * m21 * m33 + m12 * m31 * m23 + m13 * m21 * m32 - m13 * m31 * m22;

	inverted_determinant = 1.0f / (m11 * inverse.v[0] + m21 * inverse.v[4] + m31 * inverse.v[8] + m41 * inverse.v[12]);

	inverse.v[0] = inverse.v[0] * inverted_determinant;
	inverse.v[1] = inverse.v[1] * inverted_determinant;
	inverse.v[2] = inverse.v[2] * inverted_determinant;
	inverse.v[3] = inverse.v[3] * inverted_determinant;
	inverse.v[4] = inverse.v[4] * inverted_determinant;
	inverse.v[5] = inverse.v[5] * inverted_determinant;
	inverse.v[6] = inverse.v[6] * inverted_determinant;
	inverse.v[7] = inverse.v[7] * inverted_determinant;
	inverse.v[8] = inverse.v[8] * inverted_determinant;
	inverse.v[9] = inverse.v[9] * inverted_determinant;
	inverse.v[10] = inverse.v[10] * inverted_determinant;
	inverse.v[11] = inverse.v[11] * inverted_determinant;
	inverse.v[12] = inverse.v[12] * inverted_determinant;
	inverse.v[13] = inverse.v[13] * inverted_determinant;
	inverse.v[14] = inverse.v[14] * inverted_determinant;
	inverse.v[15] = inverse.v[15] * inverted_determinant;
	return inverse;
}

mat4 mat4_translate(mat4 mat, vec3 v)
{
    mat4 result;

    result.v[0] = mat.v[0];
    result.v[1] = mat.v[1];
    result.v[2] = mat.v[2];
    result.v[3] = mat.v[3];
    result.v[4] = mat.v[4];
    result.v[5] = mat.v[5];
    result.v[6] = mat.v[6];
    result.v[7] = mat.v[7];
    result.v[8] = mat.v[8];
    result.v[9] = mat.v[9];
    result.v[10] = mat.v[10];
    result.v[11] = mat.v[11];
    result.v[12] = mat.v[12] + v.x;
    result.v[13] = mat.v[13] + v.y;
    result.v[14] = mat.v[14] + v.z;
    result.v[15] = mat.v[15];

    return result;
}

mat4 mat4_rotate(mat4 mat, vec3 axis, float angle)
{
    mat4 result;

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

    result.v[0] = (xx + (yy + zz) * c) / l;
    result.v[1] = (xy * one_c + z * sqrt_l * s) / l;
    result.v[2] = (xz * one_c - y * sqrt_l * s) / l;
    result.v[3] = 0.0f;
    result.v[4] = (xy * one_c - z * sqrt_l * s) / l;
    result.v[5] = (yy + (xx + zz) * c) / l;
    result.v[6] = (yz * one_c + x * sqrt_l * s) / l;
    result.v[7] = 0.0f;
    result.v[8] = (xz * one_c + y * sqrt_l * s) / l;
    result.v[9] = (yz * one_c - x * sqrt_l * s) / l;
    result.v[10] = (zz + (xx + yy) * c) / l;
    result.v[11] = 0.0f;
    result.v[12] = 0.0f;
    result.v[13] = 0.0f;
    result.v[14] = 0.0f;
    result.v[15] = 1.0f;

    return result;
}

mat4 mat4_scale(mat4 mat, vec3 v)
{
    mat4 result;

    result.v[0] = mat.v[0] * v.x;
    result.v[1] = mat.v[1];
    result.v[2] = mat.v[2];
    result.v[3] = mat.v[3];
    result.v[4] = mat.v[4];
    result.v[5] = mat.v[5] * v.y;
    result.v[6] = mat.v[6];
    result.v[7] = mat.v[7];
    result.v[8] = mat.v[8];
    result.v[9] = mat.v[9];
    result.v[10] = mat.v[10] * v.z;
    result.v[11] = mat.v[11];
    result.v[12] = mat.v[12];
    result.v[13] = mat.v[13];
    result.v[14] = mat.v[14];
    result.v[15] = mat.v[15];
    return result;
}

mat4 mat4_multiply(mat4 left, mat4 right)
{
    mat4 result;
    result.v[0] = left.v[0] * right.v[0] + left.v[4] * right.v[1] + left.v[8] * right.v[2] + left.v[12] * right.v[3];
    result.v[1] = left.v[1] * right.v[0] + left.v[5] * right.v[1] + left.v[9] * right.v[2] + left.v[13] * right.v[3];
    result.v[2] = left.v[2] * right.v[0] + left.v[6] * right.v[1] + left.v[10] * right.v[2] + left.v[14] * right.v[3];
    result.v[3] = left.v[3] * right.v[0] + left.v[7] * right.v[1] + left.v[11] * right.v[2] + left.v[15] * right.v[3];
    result.v[4] = left.v[0] * right.v[4] + left.v[4] * right.v[5] + left.v[8] * right.v[6] + left.v[12] * right.v[7];
    result.v[5] = left.v[1] * right.v[4] + left.v[5] * right.v[5] + left.v[9] * right.v[6] + left.v[13] * right.v[7];
    result.v[6] = left.v[2] * right.v[4] + left.v[6] * right.v[5] + left.v[10] * right.v[6] + left.v[14] * right.v[7];
    result.v[7] = left.v[3] * right.v[4] + left.v[7] * right.v[5] + left.v[11] * right.v[6] + left.v[15] * right.v[7];
    result.v[8] = left.v[0] * right.v[8] + left.v[4] * right.v[9] + left.v[8] * right.v[10] + left.v[12] * right.v[11];
    result.v[9] = left.v[1] * right.v[8] + left.v[5] * right.v[9] + left.v[9] * right.v[10] + left.v[13] * right.v[11];
    result.v[10] = left.v[2] * right.v[8] + left.v[6] * right.v[9] + left.v[10] * right.v[10] + left.v[14] * right.v[11];
    result.v[11] = left.v[3] * right.v[8] + left.v[7] * right.v[9] + left.v[11] * right.v[10] + left.v[15] * right.v[11];
    result.v[12] = left.v[0] * right.v[12] + left.v[4] * right.v[13] + left.v[8] * right.v[14] + left.v[12] * right.v[15];
    result.v[13] = left.v[1] * right.v[12] + left.v[5] * right.v[13] + left.v[9] * right.v[14] + left.v[13] * right.v[15];
    result.v[14] = left.v[2] * right.v[12] + left.v[6] * right.v[13] + left.v[10] * right.v[14] + left.v[14] * right.v[15];
    result.v[15] = left.v[3] * right.v[12] + left.v[7] * right.v[13] + left.v[11] * right.v[14] + left.v[15] * right.v[15];
    return result;
}

mat4 mat4_frustum(float left, float right, float bottom, float top, float near, float far)
{
    mat4 result;

    float rl = (float)(right - left);
    float tb = (float)(top - bottom);
    float fn = (float)(far - near);

    result.v[0] = ((float)near * 2.0f) / rl;
    result.v[1] = 0.0f;
    result.v[2] = 0.0f;
    result.v[3] = 0.0f;
    result.v[4] = 0.0f;
    result.v[5] = ((float)near * 2.0f) / tb;
    result.v[6] = 0.0f;
    result.v[7] = 0.0f;
    result.v[8] = ((float)right + (float)left) / rl;
    result.v[9] = ((float)top + (float)bottom) / tb;
    result.v[10] = -((float)far + (float)near) / fn;
    result.v[11] = -1.0f;
    result.v[12] = 0.0f;
    result.v[13] = 0.0f;
    result.v[14] = -((float)far * (float)near * 2.0f) / fn;
    result.v[15] = 0.0f;

    return result;
}

mat4 mat4_perspective(float fov_y, float aspect, float near, float far)
{
    mat4 result;

    float tan_half_fov_y = 1.0f / tanf(fov_y * 0.5f);
    result.v[0] = 1.0f / aspect * tan_half_fov_y;
    result.v[1] = 0.0f;
    result.v[2] = 0.0f;
    result.v[3] = 0.0f;
    result.v[4] = 0.0f;
    result.v[5] = 1.0f / tan_half_fov_y;
    result.v[6] = 0.0f;
    result.v[7] = 0.0f;
    result.v[8] = 0.0f;
    result.v[9] = 0.0f;
    result.v[10] = far / (near - far);
    result.v[11] = -1.0f;
    result.v[12] = 0.0f;
    result.v[13] = 0.0f;
    result.v[14] = -(far * near) / (far - near);
    result.v[15] = 0.0f;
    return result;

}

mat4 mat4_ortho(float left, float right, float bottom, float top, float near, float far)
{
    mat4 result;

    float rl = 1.0f / (right - left);
    float tb = 1.0f / (top - bottom);
    float fn = -1.0f / (far - near);

    result.v[0] = 2.0f * rl;
    result.v[1] = 0.0f;
    result.v[2] = 0.0f;
    result.v[3] = 0.0f;
    result.v[4] = 0.0f;
    result.v[5] = 2.0f * tb;
    result.v[6] = 0.0f;
    result.v[7] = 0.0f;
    result.v[8] = 0.0f;
    result.v[9] = 0.0f;
    result.v[10] = 2.0f * fn;
    result.v[11] = 0.0f;
    result.v[12] = -(left + right) * rl;
    result.v[13] = -(top + bottom) * tb;
    result.v[14] = (far + near) * fn;
    result.v[15] = 1.0f;

    return result;
}

mat4 mat4_look_at(vec3 position, vec3 target, vec3 up)
{
    mat4 result;

    vec3 forward = vec3_sub(target, position);
    forward = vec3_normalize(forward);
    vec3 side = vec3_cross(forward, up);
    side = vec3_normalize(side);
    vec3 upward = vec3_cross(side, forward);
    result.v[0] = side.x;
    result.v[1] = upward.x;
    result.v[2] = -forward.x;
    result.v[3] = 0.0f;
    result.v[4] = side.y;
    result.v[5] = upward.y;
    result.v[6] = -forward.y;
    result.v[7] = 0.0f;
    result.v[8] = side.z;
    result.v[9] = upward.z;
    result.v[10] = -forward.z;
    result.v[11] = 0.0f;
    result.v[12] = -vec3_dot(side, position);
    result.v[13] = -vec3_dot(upward, position);
    result.v[14] = vec3_dot(forward, position);
    result.v[15] = 1.0f;

    return result;
}