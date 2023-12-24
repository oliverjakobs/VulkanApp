#ifndef IGNIS_COMMON_H
#define IGNIS_COMMON_H

#include <stdint.h>

#ifdef _DEBUG
    #define IGNIS_DEBUG
#endif

#define IGNIS_FAIL    0
#define IGNIS_OK      1

void* ignisAlloc(size_t size);
void  ignisFree(void* block, size_t size);


uint32_t ignisClamp32(uint32_t val, uint32_t min, uint32_t max);

#endif /* !IGNIS_COMMON_H */
