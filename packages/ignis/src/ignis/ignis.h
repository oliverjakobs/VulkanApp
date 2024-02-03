#ifndef IGNIS_H
#define IGNIS_H

#include "ignis_core.h"

/* --------------------------| platform |-------------------------------- */
typedef VkResult(*ignisCreateSurfaceFn)(VkInstance, const void*, const VkAllocationCallbacks*, VkSurfaceKHR*);
typedef const char* const* (*ignisQueryExtensionFn)(uint32_t*);
typedef struct
{
    ignisCreateSurfaceFn createSurface;
    ignisQueryExtensionFn queryExtensions;
    const void* context;
} IgnisPlatform;

uint8_t ignisInit(const char* name, const IgnisPlatform* platform);
void ignisTerminate();


#endif /* !IGNIS_H */
