#ifndef BUFFER_H
#define BUFFER_H

#include "common.h"

typedef struct {
    VkBuffer handle;
    VkDeviceMemory memory;
    VkDeviceSize size;
} Buffer;


int createVertexBuffer(const VulkanContext* context, Buffer* buffer, const void* vertices, VkDeviceSize size);
int createIndexBuffer(const VulkanContext* context, Buffer* buffer, const uint16_t* indices, uint32_t count);
int createUniformBuffer(const VulkanContext* context, Buffer* buffer, VkDeviceSize size);
void destroyBuffer(const VulkanContext* context, Buffer* buffer);

void writeBuffer(const VulkanContext* context, Buffer* buffer, const void* src, VkDeviceSize size);

#endif // !BUFFER_H
