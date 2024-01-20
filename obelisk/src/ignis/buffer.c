#include "buffer.h"


#include "minimal/common.h"

uint8_t ignisCreateVertexBuffer(const IgnisDevice* device, const float* vertices, size_t size, IgnisBuffer* buffer)
{
    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = sizeof(float) * size,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    if (vkCreateBuffer(device->handle, &bufferInfo, ignisGetAllocator(), &buffer->handle) != VK_SUCCESS)
    {
        return IGNIS_FAIL;
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device->handle, buffer->handle, &memRequirements);

    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    if (!ignisAllocateDeviceMemory(device, memRequirements, properties, &buffer->memory))
    {
        MINIMAL_ERROR("failed to find allocate device memory!");
        return IGNIS_FAIL;
    }

    vkBindBufferMemory(device->handle, buffer->handle, buffer->memory, 0);

    void* data;
    vkMapMemory(device->handle, buffer->memory, 0, bufferInfo.size, 0, &data);
        memcpy(data, vertices, (size_t) bufferInfo.size);
    vkUnmapMemory(device->handle, buffer->memory);

    return IGNIS_OK;
}

void ignisDestroyBuffer(VkDevice device, IgnisBuffer* buffer)
{
    vkDestroyBuffer(device, buffer->handle, ignisGetAllocator());
    vkFreeMemory(device, buffer->memory, ignisGetAllocator());
}