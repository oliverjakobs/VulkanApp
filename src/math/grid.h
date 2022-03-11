#ifndef GRID_H
#define GRID_H

#include "vec2.h"

float grid_clip(float granularity, float f);
float grid_clip_down(float granularity, float f);

vec2 grid_clip_vec2(float granularity, vec2 v);
vec2 grid_clip_down_vec2(float granularity, vec2 v);

#endif /* !GRID_H */