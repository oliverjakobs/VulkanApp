#ifndef VEC2_H
#define VEC2_H

typedef struct
{
	float x;
	float y;
} vec2;

vec2 vec2_zero();

vec2 vec2_mult(vec2 v, float f);
vec2 vec2_div(vec2 v, float f);
vec2 vec2_add(vec2 a, vec2 b);
vec2 vec2_sub(vec2 a, vec2 b);

vec2 vec2_normalize(vec2 v);

float vec2_dot(vec2 a, vec2 b);

int vec2_inside(vec2 point, vec2 min, vec2 max);

float vec2_distance(vec2 a, vec2 b);

typedef struct
{
	int x;
	int y;
} vec2i;

vec2i vec2i_zero();

#endif /* !VEC2_H */
