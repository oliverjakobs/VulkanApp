#ifndef IGNIS_BUFFER_H
#define IGNIS_BUFFER_H

#include "ignis_core.h"

typedef struct
{
    VkBuffer handle;
    VkDeviceMemory memory;
} IgnisBuffer;

uint8_t ignisCreateBuffer(const void* data, size_t size, VkBufferUsageFlags usage, IgnisBuffer* buffer);
uint8_t ignisCreateBufferStaged(const void* data, size_t size, VkBufferUsageFlags usage, IgnisBuffer* buffer);

#define ignisCreateVertexBuffer(vertices, size, buffer) \
    ignisCreateBufferStaged(vertices, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, buffer)

#define ignisCreateIndexBuffer(indices, count, buffer) \
    ignisCreateBufferStaged(indices, count * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, buffer)

void ignisDestroyBuffer(IgnisBuffer* buffer);

uint8_t ignisWriteBuffer(const void* data, size_t size, IgnisBuffer* buffer);

#endif /* !IGNIS_BUFFER_H */