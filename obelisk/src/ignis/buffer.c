#include "buffer.h"

#include "ignis.h"

#include "minimal/common.h"

static uint8_t ignisWriteBuffer(const void* data, size_t size, IgnisBuffer* buffer)
{
    VkDevice device = ignisGetVkDevice();

    void* mapped;
    vkMapMemory(device, buffer->memory, 0, size, 0, &mapped);

    memcpy(mapped, data, size);

    vkUnmapMemory(device, buffer->memory);
    
    return IGNIS_OK;
}

static uint8_t ignisCreateBuffer(const void* data, size_t size, VkBufferUsageFlags usage, IgnisBuffer* buffer)
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
        MINIMAL_ERROR("failed to create buffer handle!");
        return IGNIS_FAIL;
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer->handle, &memRequirements);

    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    buffer->memory = ignisAllocateDeviceMemory(memRequirements, properties, allocator);
    if (!buffer->memory)
    {
        MINIMAL_ERROR("failed to find allocate device memory!");
        return IGNIS_FAIL;
    }

    vkBindBufferMemory(device, buffer->handle, buffer->memory, 0);

    return ignisWriteBuffer(data, size, buffer);
}

uint8_t ignisCreateVertexBuffer(const float* vertices, size_t size, IgnisBuffer* buffer)
{
    if (!ignisCreateBuffer(vertices, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, buffer))
        return IGNIS_FAIL;

    return IGNIS_OK;
}

uint8_t ignisCreateIndexBuffer(const uint32_t* indices, size_t count, IgnisBuffer* buffer)
{
    if (!ignisCreateBuffer(indices, count * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, buffer))
        return IGNIS_FAIL;

    return IGNIS_OK;
}

void ignisDestroyBuffer(IgnisBuffer* buffer)
{
    VkDevice device = ignisGetVkDevice();
    const VkAllocationCallbacks* allocator = ignisGetAllocator();

    vkDestroyBuffer(device, buffer->handle, allocator);
    vkFreeMemory(device, buffer->memory, allocator);
}