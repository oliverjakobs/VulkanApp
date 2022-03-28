#include "Buffer.h"

#include "Core.h"

#include <string.h>

static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memoryProps;
    vkGetPhysicalDeviceMemoryProperties(obeliskGetPhysicalDevice(), &memoryProps);

    for (uint32_t i = 0; i < memoryProps.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memoryProps.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    MINIMAL_ERROR("failed to find suitable memory type!");
    return 0;
}

int createBuffer(Buffer* buffer, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    if (vkCreateBuffer(obeliskGetDevice(), &bufferInfo, NULL, &buffer->handle) != VK_SUCCESS) {
        MINIMAL_ERROR("failed to create buffer!");
        return MINIMAL_FAIL;
    }

    VkMemoryRequirements memoryReq;
    vkGetBufferMemoryRequirements(obeliskGetDevice(), buffer->handle, &memoryReq);

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memoryReq.size,
        .memoryTypeIndex = findMemoryType(memoryReq.memoryTypeBits, properties)
    };

    if (vkAllocateMemory(obeliskGetDevice(), &allocInfo, NULL, &buffer->memory) != VK_SUCCESS) {
        MINIMAL_ERROR("failed to allocate buffer memory!");
        return MINIMAL_FAIL;
    }

    vkBindBufferMemory(obeliskGetDevice(), buffer->handle, buffer->memory, 0);
    buffer->size = size;
    return MINIMAL_OK;
}

void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer;
    obeliskAllocateCommandBuffers(&commandBuffer, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);

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

    vkQueueSubmit(obeliskGetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(obeliskGetGraphicsQueue());

    obeliskFreeCommandBuffers(&commandBuffer, 1);
}

int createVertexBuffer(Buffer* buffer, const void* vertices, VkDeviceSize size) {
    Buffer staging = { 0 };
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    if (!createBuffer(&staging, size, usage, properties))
        return MINIMAL_FAIL;

    writeBuffer(&staging, vertices, size);

    usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    if (!createBuffer(buffer, size, usage, properties)) {
        destroyBuffer(&staging);
        return MINIMAL_FAIL;
    }

    copyBuffer(staging.handle, buffer->handle, size);
    destroyBuffer(&staging);

    return MINIMAL_OK;
}

int createIndexBuffer(Buffer* buffer, const uint16_t* indices, uint32_t count) {
    VkDeviceSize size = count * sizeof(*indices);

    Buffer staging = { 0 };
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    if (!createBuffer(&staging, size, usage, properties))
        return MINIMAL_FAIL;

    writeBuffer(&staging, indices, size);

    usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    if (!createBuffer(buffer, size, usage, properties)) {
        destroyBuffer(&staging);
        return MINIMAL_FAIL;
    }

    copyBuffer(staging.handle, buffer->handle, size);
    destroyBuffer(&staging);

    return MINIMAL_OK;
}

int createUniformBuffer(Buffer* buffer, VkDeviceSize size) {
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    return createBuffer(buffer, size, usage, properties);
}

void destroyBuffer(Buffer* buffer) {
    vkDestroyBuffer(obeliskGetDevice(), buffer->handle, NULL);
    vkFreeMemory(obeliskGetDevice(), buffer->memory, NULL);
}

void writeBuffer(Buffer* buffer, const void* src, VkDeviceSize size) {
    void* dst;
    vkMapMemory(obeliskGetDevice(), buffer->memory, 0, size, 0, &dst);
    memcpy(dst, src, (size_t)size);
    vkUnmapMemory(obeliskGetDevice(), buffer->memory);
}
