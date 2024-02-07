#ifndef IGNIS_TEXTURE_H
#define IGNIS_TEXTURE_H

#include "ignis_core.h"

typedef struct
{
    VkImage image;
    VkImageView view;
    VkSampler sampler;

    VkDeviceMemory memory;

    int width, height;
} IgnisTexture;


uint8_t ignisCreateTexture(const char* path, IgnisTexture* texture);
void ignisDestroyTexture(IgnisTexture* texture);

#endif // !IGNIS_TEXTURE_H