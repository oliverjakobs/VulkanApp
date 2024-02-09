#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

#include "buffer.h"

static uint8_t ignisTransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .subresourceRange.baseMipLevel = 0,
        .subresourceRange.levelCount = 1,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.layerCount = 1
    };

    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_NONE_KHR;
    VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_NONE_KHR;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED
        && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        IGNIS_WARN("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, NULL, 0, NULL, 1, &barrier);

    return IGNIS_OK;
}

uint8_t ignisCreateTexture(const char* path, IgnisTexture* texture)
{
    VkDevice device = ignisGetVkDevice();
    const VkAllocationCallbacks* allocator = ignisGetAllocator();

    size_t dataSize;
    char* data = ignisReadFile(path, &dataSize);

    if (!data)
    {
        IGNIS_ERROR("[Texture] Failed to read image file: %s", path);
        return IGNIS_FAIL;
    }

    //stbi_set_flip_vertically_on_load(config.flip_on_load);

    int bpp = 0;
    int width, height;
    uint8_t* pixels = stbi_load_from_memory(data, (int)dataSize, &width, &height, &bpp, STBI_rgb_alpha);

    if (!pixels)
    {
        IGNIS_ERROR("[Texture] Failed to load texture: %s", stbi_failure_reason());
        return IGNIS_FAIL;
    }

    VkDeviceSize imageSize = (VkDeviceSize)width * height * 4;

    IgnisBuffer stagingBuffer;
    ignisCreateBuffer(pixels, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, &stagingBuffer);

    stbi_image_free(pixels);

    texture->extent = (VkExtent3D){
        .width = width,
        .height = height,
        .depth = 1
    };

    VkImageCreateInfo imageInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .extent = texture->extent,
        .mipLevels = 1,
        .arrayLayers = 1,
        .format = VK_FORMAT_R8G8B8A8_SRGB,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    if (vkCreateImage(device, &imageInfo, allocator, &texture->image) != VK_SUCCESS)
    {
        IGNIS_ERROR("failed to create image!");
        return IGNIS_FAIL;
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, texture->image, &memRequirements);

    texture->memory = ignisAllocateDeviceMemory(memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, allocator);

    vkBindImageMemory(device, texture->image, texture->memory, 0);

    /* copy buffer to image */
    VkCommandBuffer commandBuffer = ignisBeginOneTimeCommandBuffer();

    ignisTransitionImageLayout(commandBuffer, texture->image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkBufferImageCopy region = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .imageSubresource.mipLevel = 0,
        .imageSubresource.baseArrayLayer = 0,
        .imageSubresource.layerCount = 1,
        .imageOffset = { 0, 0, 0 },
        .imageExtent = texture->extent
    };

    vkCmdCopyBufferToImage(commandBuffer, stagingBuffer.handle, texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    ignisTransitionImageLayout(commandBuffer, texture->image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    ignisEndOneTimeCommandBuffer(commandBuffer);

    ignisDestroyBuffer(&stagingBuffer);

    /* create image view */
    VkImageViewCreateInfo viewInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = texture->image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_SRGB,
        .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .subresourceRange.baseMipLevel = 0,
        .subresourceRange.levelCount = 1,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.layerCount = 1
    };

    if (vkCreateImageView(device, &viewInfo, allocator, &texture->view) != VK_SUCCESS)
    {
        IGNIS_ERROR("failed to create texture image view!");
        return IGNIS_FAIL;
    }

    /* create sampler */
    VkPhysicalDeviceProperties properties = { 0 };
    vkGetPhysicalDeviceProperties(ignisGetVkPhysicalDevice(), &properties);

    VkSamplerCreateInfo samplerInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .mipLodBias = 0.0f,
        .minLod = 0.0f,
        .maxLod = 0.0f,
    };

    if (vkCreateSampler(device, &samplerInfo, allocator, &texture->sampler) != VK_SUCCESS)
    {
        IGNIS_ERROR("failed to create texture sampler!");
        return IGNIS_FAIL;
    }

    return IGNIS_OK;
}

void ignisDestroyTexture(IgnisTexture* texture)
{
    VkDevice device = ignisGetVkDevice();
    const VkAllocationCallbacks* allocator = ignisGetAllocator();

    vkDestroySampler(device, texture->sampler, allocator);
    vkDestroyImageView(device, texture->view, allocator);

    vkDestroyImage(device, texture->image, allocator);
    vkFreeMemory(device, texture->memory, allocator);
}