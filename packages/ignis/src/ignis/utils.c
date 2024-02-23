#include "utils.h"

VkSurfaceFormatKHR ignisChooseSurfaceFormat(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    VkSurfaceFormatKHR wantedFormat = {
        .format = VK_FORMAT_B8G8R8A8_UNORM,
        .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
    };

    uint32_t count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, NULL);
    if (!count) return (VkSurfaceFormatKHR) { VK_FORMAT_UNDEFINED, 0 };

    VkSurfaceFormatKHR* formats = ignisAlloc(sizeof(VkSurfaceFormatKHR) * count);
    if (!formats) return (VkSurfaceFormatKHR) { VK_FORMAT_UNDEFINED, 0 };

    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, formats);

    /* set fallback */
    VkSurfaceFormatKHR format = formats[0];
    for (uint32_t i = 0; i < count; ++i)
    {
        if (formats[i].format == wantedFormat.format && formats[i].colorSpace == wantedFormat.colorSpace)
        {
            format = formats[i];
            break;
        }
    }

    ignisFree(formats, sizeof(VkSurfaceFormatKHR) * count);
    return format;
}

VkPresentModeKHR ignisChoosePresentMode(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    VkPresentModeKHR wantedMode = VK_PRESENT_MODE_MAILBOX_KHR;
    //VkPresentModeKHR wantedMode = VK_PRESENT_MODE_FIFO_KHR;

    uint32_t count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, NULL);
    if (!count) return VK_PRESENT_MODE_FIFO_KHR;

    VkPresentModeKHR* modes = ignisAlloc(sizeof(VkPresentModeKHR) * count);
    if (!modes) return VK_PRESENT_MODE_FIFO_KHR;

    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, modes);

    /* set fallback */
    VkPresentModeKHR mode = VK_PRESENT_MODE_FIFO_KHR;
    for (uint32_t i = 0; i < count; ++i)
    {
        if (modes[i] == wantedMode)
        {
            mode = modes[i];
            break;
        }
    }

    ignisFree(modes, sizeof(VkPresentModeKHR) * count);
    return mode;
}

VkFormat ignisQueryDepthFormat(VkPhysicalDevice device)
{
    VkFormat candidates[] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };
    const uint32_t count = sizeof(candidates) / sizeof(candidates[0]);;

    VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VkFormat format = VK_FORMAT_UNDEFINED;
    for (uint32_t i = 0; i < count; ++i)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(device, candidates[i], &props);

        if ((props.linearTilingFeatures & features) == features)
        {
            format = candidates[i];
            break;
        }
        else if ((props.optimalTilingFeatures & features) == features)
        {
            format = candidates[i];
            break;
        }
    }

    return format;
}

uint8_t ignisCheckValidationLayerSupport(const char* const* layers, uint32_t layerCount)
{
    uint32_t count;
    vkEnumerateInstanceLayerProperties(&count, NULL);
    if (!count || layerCount > count) return IGNIS_FAIL;

    VkLayerProperties* properties = ignisAlloc(sizeof(VkLayerProperties) * count);
    if (!properties) return IGNIS_FAIL;

    vkEnumerateInstanceLayerProperties(&count, properties);

    uint8_t found = IGNIS_FAIL;
    for (size_t i = 0; i < layerCount; ++i)
    {
        found = IGNIS_FAIL;
        for (size_t j = 0; j < count; ++j)
        {
            if (strcmp(layers[i], properties[j].layerName) == 0)
            {
                found = IGNIS_OK;
                break;
            }
        }

        if (!found) break;
    }

    ignisFree(properties, sizeof(VkLayerProperties) * count);
    return found;
}


uint8_t ignisCheckDeviceExtensionSupport(VkPhysicalDevice device, const char* const* extensions, uint32_t extensionCount)
{
    uint32_t count = 0;
    vkEnumerateDeviceExtensionProperties(device, NULL, &count, NULL);
    if (!count || extensionCount > count) return IGNIS_FAIL;

    VkExtensionProperties* properties = ignisAlloc(sizeof(VkExtensionProperties) * count);
    if (!properties) return IGNIS_FAIL;

    vkEnumerateDeviceExtensionProperties(device, NULL, &count, properties);

    uint8_t found = IGNIS_FAIL;
    for (size_t i = 0; i < extensionCount; ++i)
    {
        found = IGNIS_FAIL;
        for (size_t j = 0; j < count; ++j)
        {
            if (strcmp(extensions[i], properties[j].extensionName) == 0)
            {
                found = IGNIS_OK;
                break;
            }
        }
        if (!found) break;
    }

    ignisFree(properties, sizeof(VkExtensionProperties) * count);
    return found;
}



VkExtent2D ignisClampExtent2D(VkExtent2D extent, VkExtent2D min, VkExtent2D max)
{
    extent.width = ignisClamp32(extent.width, min.width, max.width);
    extent.height = ignisClamp32(extent.height, min.height, max.height);
    return extent;
}