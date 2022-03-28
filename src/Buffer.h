#ifndef BUFFER_H
#define BUFFER_H

#include "common.h"

typedef struct {
    VkBuffer handle;
    VkDeviceMemory memory;
    VkDeviceSize size;
} Buffer;

int createVertexBuffer(Buffer* buffer, const void* vertices, VkDeviceSize size);
int createIndexBuffer(Buffer* buffer, const uint16_t* indices, uint32_t count);
int createUniformBuffer(Buffer* buffer, VkDeviceSize size);
void destroyBuffer(Buffer* buffer);

void writeBuffer(Buffer* buffer, const void* src, VkDeviceSize size);

#endif // !BUFFER_H
