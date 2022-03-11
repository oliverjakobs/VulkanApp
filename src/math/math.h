#ifndef MATH_H
#define MATH_H

#include <math.h>
#include <stdint.h>

/* based on https://github.com/felselva/mathc */
#include "vec2.h"
#include "vec3.h"

#include "mat4.h"

typedef struct
{
	vec2 min;
	vec2 max;
} rect;

typedef struct
{
	vec2 start;
	vec2 end;
} line;

#endif /* !MATH_H */
