#ifndef BUFFER_H
#define BUFFER_H

#include "common.h"

typedef struct {
    float pos[2];
    float color[3];
} Vertex;

VkVertexInputBindingDescription* getVertexBindingDescriptions(uint32_t* count);
VkVertexInputAttributeDescription* getVertexAttributeDescriptions(uint32_t* count);

typedef struct {
    VkBuffer handle;
    VkDeviceMemory memory;
    VkDeviceSize size;
} Buffer;

int createVertexBuffer(const VulkanContext* context, Buffer* buffer, const Vertex* vertices, uint32_t count);
int createIndexBuffer(const VulkanContext* context, Buffer* buffer, const uint16_t* indices, uint32_t count);
int createUniformBuffer(const VulkanContext* context, Buffer* buffer, VkDeviceSize size);

void writeBuffer(const VulkanContext* context, Buffer* buffer, const void* src, VkDeviceSize size);

void destroyBuffer(const VulkanContext* context, Buffer* buffer);

#endif // !BUFFER_H
