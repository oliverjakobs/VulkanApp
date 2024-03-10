#ifndef IGNIS_TEXTURE_H
#define IGNIS_TEXTURE_H

#include "ignis_core.h"

typedef struct
{
    VkFormat format;

    VkFilter minFilter;
    VkFilter magFilter;

    VkSamplerAddressMode addressMode;
} IgnisTextureConfig;

#define IGNIS_DEFAULT_CONFIG (IgnisTextureConfig){ VK_FORMAT_R8G8B8A8_SRGB, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT }

typedef struct
{
    VkImage image;
    VkImageView view;
    VkDeviceMemory memory;

    VkSampler sampler;
    VkExtent3D extent;
} IgnisTexture;


uint8_t ignisCreateTexture(const void* pixels, uint32_t width, uint32_t height, IgnisTextureConfig* configPtr, IgnisTexture* texture);
void ignisDestroyTexture(IgnisTexture* texture);

uint8_t ignisLoadTexture(const char* path, IgnisTextureConfig* configPtr, uint8_t flipOnLoad, IgnisTexture* texture);

uint8_t ignisTransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldLayout, VkImageLayout newLayout);

#endif // !IGNIS_TEXTURE_H