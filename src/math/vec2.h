#ifndef VEC2_H
#define VEC2_H

#include <stdint.h>

typedef struct
{
    float x;
    float y;
} vec2;

vec2 vec2_mult(vec2 v, float f);
vec2 vec2_div(vec2 v, float f);
vec2 vec2_add(vec2 a, vec2 b);
vec2 vec2_sub(vec2 a, vec2 b);

vec2 vec2_normalize(vec2 v);

float vec2_dot(vec2 a, vec2 b);

typedef struct
{
    uint32_t x;
    uint32_t y;
} vec2i;

#endif /* !VEC2_H */
