#include "vec2.h"

#include <math.h>

vec2 vec2_zero()
{
	return (vec2) {0.0f, 0.0f};
}

vec2 vec2_mult(vec2 vec, float f)
{
	vec2 v;
	v.x = vec.x * f;
	v.y = vec.y * f;

	return v;
}

vec2 vec2_div(vec2 vec, float f)
{
	vec2 v;
	v.x = vec.x / f;
	v.y = vec.y / f;

	return v;
}

vec2 vec2_add(vec2 a, vec2 b)
{
	vec2 v;
	v.x = a.x + b.x;
	v.y = a.y + b.y;

	return v;
}

vec2 vec2_sub(vec2 a, vec2 b)
{
	vec2 v;
	v.x = a.x - b.x;
	v.y = a.y - b.y;

	return v;
}

vec2 vec2_normalize(vec2 v)
{
	vec2 result;

	float l = sqrtf(v.x * v.x + v.y * v.y);
	result.x = v.x / l;
	result.y = v.y / l;

	return v;
}

float vec2_dot(vec2 a, vec2 b)
{
	return a.x * b.x + a.y * b.y;
}

int vec2_inside(vec2 point, vec2 min, vec2 max)
{
	if (point.x < min.x || point.y < min.y) return 0;
	if (point.x > max.x || point.y > max.y) return 0;

	return 1;
}

float vec2_distance(vec2 a, vec2 b)
{
	vec2 dis_vec;
	dis_vec.x = a.x - b.x;
	dis_vec.y = a.y - b.y;
	return sqrtf(vec2_dot(dis_vec, dis_vec));
}

vec2i vec2i_zero()
{
	return (vec2i){0,0};
}
