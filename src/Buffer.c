#include "Buffer.h"

#include "Core.h"

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
        MINIMAL_ERROR("failed to allocate buffer memory!");
        return MINIMAL_FAIL;
    }

    vkBindBufferMemory(context->device, buffer->handle, buffer->memory, 0);
    buffer->size = size;
    return MINIMAL_OK;
}

void copyBuffer(const VulkanContext* context, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = context->commandPool,
        .commandBufferCount = 1
    };

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(context->device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion = { .size = size };
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pCommandBuffers = &commandBuffer,
        .commandBufferCount = 1
    };

    vkQueueSubmit(context->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(context->graphicsQueue);

    vkFreeCommandBuffers(context->device, context->commandPool, 1, &commandBuffer);
}

int createVertexBuffer(const VulkanContext* context, Buffer* buffer, const Vertex* vertices, uint32_t count) {
    VkDeviceSize size = count * sizeof(*vertices);

    Buffer staging = { 0 };
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    if (!createBuffer(context, &staging, size, usage, properties))
        return MINIMAL_FAIL;

    void* data;
    vkMapMemory(context->device, staging.memory, 0, size, 0, &data);
    memcpy(data, vertices, (size_t)size);
    vkUnmapMemory(context->device, staging.memory);

    usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    if (!createBuffer(context, buffer, size, usage, properties)) {
        destroyBuffer(context, &staging);
        return MINIMAL_FAIL;
    }

    copyBuffer(context, staging.handle, buffer->handle, size);
    destroyBuffer(context, &staging);

    return MINIMAL_OK;
}

int createIndexBuffer(const VulkanContext* context, Buffer* buffer, const uint16_t* indices, uint32_t count) {
    VkDeviceSize size = count * sizeof(*indices);

    Buffer staging = { 0 };
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    if (!createBuffer(context, &staging, size, usage, properties))
        return MINIMAL_FAIL;

    void* data;
    vkMapMemory(context->device, staging.memory, 0, size, 0, &data);
    memcpy(data, indices, (size_t)size);
    vkUnmapMemory(context->device, staging.memory);

    usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    if (!createBuffer(context, buffer, size, usage, properties)) {
        destroyBuffer(context, &staging);
        return MINIMAL_FAIL;
    }

    copyBuffer(context, staging.handle, buffer->handle, size);
    destroyBuffer(context, &staging);

    return MINIMAL_OK;
}

int createUniformBuffer(const VulkanContext* context, Buffer* buffer, VkDeviceSize size) {
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    return createBuffer(context, buffer, size, usage, properties);
}

void writeBuffer(const VulkanContext* context, Buffer* buffer, const void* src, VkDeviceSize size) {
    void* dst;
    vkMapMemory(context->device, buffer->memory, 0, size, 0, &dst);
    memcpy(dst, src, (size_t)size);
    vkUnmapMemory(context->device, buffer->memory);
}

void destroyBuffer(const VulkanContext* context, Buffer* buffer) {
    vkDestroyBuffer(context->device, buffer->handle, NULL);
    vkFreeMemory(context->device, buffer->memory, NULL);
}
