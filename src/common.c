#include "common.h"


uint32_t clamp32(uint32_t val, uint32_t min, uint32_t max) {
    const uint32_t t = val < min ? min : val;
    return t > max ? max : t;
}