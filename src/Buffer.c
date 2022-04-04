#include "Buffer.h"

#include "Core.h"

#include <string.h>

static int obeliskCreateBuffer(ObeliskBuffer* buffer, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
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
        .memoryTypeIndex = obeliskFindMemoryTypeIndex(memoryReq.memoryTypeBits, properties)
    };

    if (vkAllocateMemory(obeliskGetDevice(), &allocInfo, NULL, &buffer->memory) != VK_SUCCESS) {
        MINIMAL_ERROR("failed to allocate buffer memory!");
        return MINIMAL_FAIL;
    }

    vkBindBufferMemory(obeliskGetDevice(), buffer->handle, buffer->memory, 0);
    return MINIMAL_OK;
}

static void obeliskCopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
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

static int obeliskStageBuffer(ObeliskBuffer* target, const void* data, VkBufferUsageFlags targetUsage) {
    VkDeviceSize size = target->size;

    ObeliskBuffer staging = { 0 };
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    if (!obeliskCreateBuffer(&staging, size, usage, properties)) {
        MINIMAL_ERROR("failed to create staging buffer!");
        return MINIMAL_FAIL;
    }

    void* dst;
    vkMapMemory(obeliskGetDevice(), staging.memory, 0, size, 0, &dst);
    memcpy(dst, data, (size_t)size);
    vkUnmapMemory(obeliskGetDevice(), staging.memory);

    usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | targetUsage;
    properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    int result = obeliskCreateBuffer(target, size, usage, properties);
    if (result) {
        obeliskCopyBuffer(staging.handle, target->handle, size);
    }

    obeliskDestroyBuffer(&staging);

    return result;
}

int obeliskCreateVertexBuffer(ObeliskBuffer* buffer, const void* vertices, uint32_t count, uint32_t vertexSize) {
    buffer->size = (VkDeviceSize)count * vertexSize;

    if (!obeliskStageBuffer(buffer, vertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)) {
        return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
}

int obeliskCreateIndexBuffer(ObeliskBuffer* buffer, const uint32_t* indices, uint32_t count) {
    buffer->size = count * sizeof(uint32_t);

    if (!obeliskStageBuffer(buffer, indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT)) {
        return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
}

int obeliskCreateUniformBuffer(ObeliskBuffer* buffer, VkDeviceSize size) {
    buffer->size = size;

    VkBufferUsageFlags usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    if (!obeliskCreateBuffer(buffer, size, usage, properties)) {
        return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
}

void obeliskDestroyBuffer(ObeliskBuffer* buffer) {
    vkDestroyBuffer(obeliskGetDevice(), buffer->handle, NULL);
    vkFreeMemory(obeliskGetDevice(), buffer->memory, NULL);
}

void obeliskWriteBuffer(ObeliskBuffer* buffer, const void* src, VkDeviceSize size) {
    MINIMAL_ASSERT(size <= buffer->size, "Buffer too small");
    void* dst;
    vkMapMemory(obeliskGetDevice(), buffer->memory, 0, size, 0, &dst);
    memcpy(dst, src, (size_t)size);
    vkUnmapMemory(obeliskGetDevice(), buffer->memory);
}
