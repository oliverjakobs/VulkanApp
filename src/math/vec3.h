#ifndef VEC3_H
#define VEC3_H

typedef struct
{
	float x;
	float y;
	float z;
} vec3;

vec3 vec3_mult(vec3 v, float f);
vec3 vec3_add(vec3 a, vec3 b);
vec3 vec3_sub(vec3 a, vec3 b);

vec3 vec3_normalize(vec3 v);
vec3 vec3_cross(vec3 left, vec3 right);

float vec3_dot(vec3 left, vec3 right);

#endif /* !VEC3_H */
