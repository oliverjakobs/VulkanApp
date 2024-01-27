#ifndef IGNIS_BUFFER_H
#define IGNIS_BUFFER_H

#include "ignis_core.h"

typedef struct
{
    VkBuffer handle;
    VkDeviceMemory memory;
} IgnisBuffer;

uint8_t ignisCreateVertexBuffer(const float* vertices, size_t size, IgnisBuffer* buffer);

void ignisDestroyBuffer(IgnisBuffer* buffer);

#endif /* !IGNIS_BUFFER_H */