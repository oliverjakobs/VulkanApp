#include "common.h"

#include <stdlib.h>

void* ignisAlloc(size_t size) { return malloc(size); }
void  ignisFree(void* block, size_t size)  { free(block); }

uint32_t ignisClamp32(uint32_t val, uint32_t min, uint32_t max)
{
    const uint32_t t = val < min ? min : val;
    return t > max ? max : t;
}