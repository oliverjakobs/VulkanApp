#ifndef MATH_H
#define MATH_H

#include <math.h>
#include <stdint.h>

#include "vec2.h"
#include "vec3.h"

#include "mat4.h"

#define MPI   3.1415926536f
#define MPI_2 1.5707963268f
#define MPI_4 0.7853981634f

inline float degToRad(float angle) { return MPI * angle / 180.f; }

#endif /* !MATH_H */
