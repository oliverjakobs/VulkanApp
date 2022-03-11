#include "Grid.h"

#include <math.h>

float grid_clip(float granularity, float f)
{
	return roundf(f / granularity) * granularity;
}

float grid_clip_down(float granularity, float f)
{
	return floorf(f / granularity) * granularity;
}

vec2 grid_clip_vec2(float granularity, vec2 v)
{
	float x = grid_clip(granularity, v.x);
	float y = grid_clip(granularity, v.y);

	return (vec2) { x, y };
}

vec2 grid_clip_down_vec2(float granularity, vec2 v)
{
	float x = grid_clip_down(granularity, v.x);
	float y = grid_clip_down(granularity, v.y);

	return (vec2) { x, y };
}
