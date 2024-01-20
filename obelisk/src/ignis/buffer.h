#ifndef IGNIS_BUFFER_H
#define IGNIS_BUFFER_H

#include "ignis_core.h"

typedef struct
{
    VkBuffer handle;
    VkDeviceMemory memory;
} IgnisBuffer;

uint8_t ignisCreateVertexBuffer(const IgnisDevice* device, const float* vertices, size_t size, IgnisBuffer* buffer);

void ignisDestroyBuffer(VkDevice device, IgnisBuffer* buffer);

#endif /* !IGNIS_BUFFER_H */