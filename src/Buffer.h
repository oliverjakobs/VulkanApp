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
} Buffer;

int createVertexBuffer(const VulkanContext* context, Buffer* buffer, const Vertex* vertecies, uint32_t count);

void destroyBuffer(const VulkanContext* context, Buffer* buffer);

#endif // !BUFFER_H
