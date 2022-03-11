#include "vec3.h"

#include <math.h>

vec3 vec3_mult(vec3 vec, float f)
{
	vec3 v;
	v.x = vec.x * f;
	v.y = vec.y * f;
	v.z = vec.z * f;

	return v;
}

vec3 vec3_add(vec3 a, vec3 b)
{
	vec3 v;
	v.x = a.x + b.x;
	v.y = a.y + b.y;
	v.z = a.z + b.z;

	return v;
}

vec3 vec3_sub(vec3 a, vec3 b)
{
	vec3 v;
	v.x = a.x - b.x;
	v.y = a.y - b.y;
	v.z = a.z - b.z;

	return v;
}

vec3 vec3_normalize(vec3 v)
{
	vec3 result;

	float l = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	result.x = v.x / l;
	result.y = v.y / l;
	result.z = v.z / l;

	return v;
}

vec3 vec3_cross(vec3 left, vec3 right)
{
	vec3 result;

	result.x = left.y * right.z - left.z * right.y;
	result.y = left.z * right.x - left.x * right.z;
	result.z = left.x * right.y - left.y * right.x;

	return result;
}

float vec3_dot(vec3 left, vec3 right)
{
	return left.x * right.x + left.y * right.y + left.z * right.z;
}
