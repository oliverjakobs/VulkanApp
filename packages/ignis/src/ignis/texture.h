#ifndef IGNIS_TEXTURE_H
#define IGNIS_TEXTURE_H

#include "ignis_core.h"

typedef struct
{
    VkFormat format;

    VkFilter minFilter;
    VkFilter magFilter;

    VkSamplerAddressMode addressMode;

    uint8_t flipOnLoad;
} IgnisTextureConfig;

#define IGNIS_DEFAULT_CONFIG (IgnisTextureConfig){ VK_FORMAT_R8G8B8A8_SRGB, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, 0 }

typedef struct
{
    VkImage image;
    VkImageView view;
    VkDeviceMemory memory;

    VkSampler sampler;
    VkExtent3D extent;
} IgnisTexture;


uint8_t ignisCreateTexture(const char* path, IgnisTexture* texture, IgnisTextureConfig* configPtr);
void ignisDestroyTexture(IgnisTexture* texture);

uint8_t ignisTransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldLayout, VkImageLayout newLayout);

#endif // !IGNIS_TEXTURE_H