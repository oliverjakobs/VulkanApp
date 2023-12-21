#ifndef IGNIS_H
#define IGNIS_H

#include "ignis_core.h"

/* --------------------------| platform |-------------------------------- */
typedef VkResult (*ignisCreateSurfaceFn)(VkInstance, const void*, const VkAllocationCallbacks*, VkSurfaceKHR*);
typedef struct
{
    ignisCreateSurfaceFn create_surface;
    const void* context;
} IgnisPlatform;

/* --------------------------| setup |----------------------------------- */
uint8_t ignisInit(const char* name, const IgnisPlatform* platform);
void ignisTerminate();


uint8_t ignisBeginFrame();
uint8_t ignisEndFrame();

#endif /* !IGNIS_H */
