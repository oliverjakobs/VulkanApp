#ifndef OBELISK_PLATFORM_H
#define OBELISK_PLATFORM_H

#include "../common.h"

typedef struct ObeliskPlatform ObeliskPlatform;

ObeliskPlatform* obeliskPlatformInit(const char* title, uint32_t w, uint32_t h);

#endif // !OBELISK_PLATFORM_H
