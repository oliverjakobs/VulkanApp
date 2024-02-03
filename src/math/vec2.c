#include "vec2.h"

#include <math.h>

vec2 vec2_mult(vec2 v, float f)
{
    vec2 result = { v.x * f, v.y * f };
    return result;
}

vec2 vec2_div(vec2 v, float f)
{
    vec2 result = { v.x / f, v.y / f };
    return result;
}

vec2 vec2_add(vec2 a, vec2 b)
{
    vec2 result = { a.x + b.x, a.y + b.y };
    return result;
}

vec2 vec2_sub(vec2 a, vec2 b)
{
    vec2 result = { a.x - b.x, a.y - b.y };
    return result;
}

vec2 vec2_normalize(vec2 v)
{
    float l = 1.0f / sqrtf(v.x * v.x + v.y * v.y);
    vec2 result = { v.x * l, v.y * l };
    return result;
}

float vec2_dot(vec2 a, vec2 b)
{
    return a.x * b.x + a.y * b.y;
}