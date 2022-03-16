#include "Buffer.h"

#include <string.h>

static VkVertexInputBindingDescription vertexBindingDescs[] = {
    {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    }
};

static VkVertexInputAttributeDescription vertexAttributeDescs[] = {
    {
        .binding = 0,
        .location = 0,
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = offsetof(Vertex, pos)
    },
    {
        .binding = 0,
        .location = 1,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(Vertex, color)
    }
};

VkVertexInputBindingDescription* getVertexBindingDescriptions(uint32_t* count) {
    *count = sizeof(vertexBindingDescs) / sizeof(vertexBindingDescs[0]);
    return vertexBindingDescs;
}

VkVertexInputAttributeDescription* getVertexAttributeDescriptions(uint32_t* count) {
    *count = sizeof(vertexAttributeDescs) / sizeof(vertexAttributeDescs[0]);
    return vertexAttributeDescs;
}

static uint32_t findMemoryType(const VulkanContext* context, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memoryProps;
    vkGetPhysicalDeviceMemoryProperties(context->physicalDevice, &memoryProps);

    for (uint32_t i = 0; i < memoryProps.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memoryProps.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    MINIMAL_ERROR("failed to find suitable memory type!");
    return 0;
}

int createBuffer(const VulkanContext* context, Buffer* buffer, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    if (vkCreateBuffer(context->device, &bufferInfo, NULL, &buffer->handle) != VK_SUCCESS) {
        MINIMAL_ERROR("failed to create buffer!");
        return MINIMAL_FAIL;
    }

    VkMemoryRequirements memoryReq;
    vkGetBufferMemoryRequirements(context->device, buffer->handle, &memoryReq);

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memoryReq.size,
        .memoryTypeIndex = findMemoryType(context, memoryReq.memoryTypeBits, properties)
    };

    if (vkAllocateMemory(context->device, &allocInfo, NULL, &buffer->memory) != VK_SUCCESS) {
        MINIMAL_ERROR("failed to allocate vertex buffer memory!");
        return MINIMAL_FAIL;
    }

    vkBindBufferMemory(context->device, buffer->handle, buffer->memory, 0);
    return MINIMAL_OK;
}

int createVertexBuffer(const VulkanContext* context, Buffer* buffer, const Vertex* vertices, uint32_t count) {
    VkDeviceSize bufferSize = count * sizeof(Vertex);
    VkMemoryPropertyFlags memPropFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    if (!createBuffer(context, buffer, bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, memPropFlags))
        return MINIMAL_FAIL;

    void* data;
    vkMapMemory(context->device, buffer->memory, 0, bufferSize, 0, &data);
    memcpy(data, vertices, (size_t)bufferSize);
    vkUnmapMemory(context->device, buffer->memory);

    return MINIMAL_OK;
}

void destroyBuffer(const VulkanContext* context, Buffer* buffer) {
    vkDestroyBuffer(context->device, buffer->handle, NULL);
    vkFreeMemory(context->device, buffer->memory, NULL);
}
