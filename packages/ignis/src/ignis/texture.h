#ifndef IGNIS_TEXTURE_H
#define IGNIS_TEXTURE_H

#include "ignis_core.h"

typedef struct
{
    VkImage image;
    VkImageView view;
    VkDeviceMemory memory;

    VkSampler sampler;
    VkExtent3D extent;
} IgnisTexture;


uint8_t ignisCreateTexture(const char* path, IgnisTexture* texture);
void ignisDestroyTexture(IgnisTexture* texture);

#endif // !IGNIS_TEXTURE_H