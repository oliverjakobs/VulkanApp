#include "buffer.h"

#include "ignis.h"


static void ignisCopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = ignisBeginOneTimeCommandBuffer();

    VkBufferCopy copyRegion = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size
    };

    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    ignisEndOneTimeCommandBuffer(commandBuffer);
}

uint8_t ignisCreateBuffer(const void* data, size_t size, VkBufferUsageFlags usage, IgnisBuffer* buffer)
{
    VkDevice device = ignisGetVkDevice();
    const VkAllocationCallbacks* allocator = ignisGetAllocator();

    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    if (vkCreateBuffer(device, &bufferInfo, allocator, &buffer->handle) != VK_SUCCESS)
    {
        IGNIS_ERROR("failed to create buffer handle!");
        return IGNIS_FAIL;
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer->handle, &memRequirements);

    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    buffer->memory = ignisAllocateDeviceMemory(memRequirements, properties, allocator);
    if (!buffer->memory)
    {
        IGNIS_ERROR("failed to find allocate device memory!");
        return IGNIS_FAIL;
    }

    vkBindBufferMemory(device, buffer->handle, buffer->memory, 0);

    if (!data) return IGNIS_OK;

    return ignisWriteBuffer(data, size, buffer);
}

uint8_t ignisCreateBufferStaged(const void* data, size_t size, VkBufferUsageFlags usage, IgnisBuffer* buffer)
{
    VkDevice device = ignisGetVkDevice();
    const VkAllocationCallbacks* allocator = ignisGetAllocator();

    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    if (vkCreateBuffer(device, &bufferInfo, allocator, &buffer->handle) != VK_SUCCESS)
    {
        IGNIS_ERROR("failed to create buffer handle!");
        return IGNIS_FAIL;
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer->handle, &memRequirements);

    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    buffer->memory = ignisAllocateDeviceMemory(memRequirements, properties, allocator);
    if (!buffer->memory)
    {
        IGNIS_ERROR("failed to find allocate device memory!");
        return IGNIS_FAIL;
    }

    vkBindBufferMemory(device, buffer->handle, buffer->memory, 0);

    IgnisBuffer stagingBuffer;
    ignisCreateBuffer(data, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, &stagingBuffer);

    ignisCopyBuffer(stagingBuffer.handle, buffer->handle, size);

    ignisDestroyBuffer(&stagingBuffer);

    return IGNIS_OK;
}

void ignisDestroyBuffer(IgnisBuffer* buffer)
{
    VkDevice device = ignisGetVkDevice();
    const VkAllocationCallbacks* allocator = ignisGetAllocator();

    vkDestroyBuffer(device, buffer->handle, allocator);
    vkFreeMemory(device, buffer->memory, allocator);
}

uint8_t ignisWriteBuffer(const void* data, size_t size, IgnisBuffer* buffer)
{
    VkDevice device = ignisGetVkDevice();

    void* mapped;
    vkMapMemory(device, buffer->memory, 0, size, 0, &mapped);

    memcpy(mapped, data, size);

    vkUnmapMemory(device, buffer->memory);

    return IGNIS_OK;
}