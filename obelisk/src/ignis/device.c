#include "device.h"

#include <string.h>

#include "minimal/minimal.h"


// Requirements
static const char* const device_extensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static const uint32_t device_extension_count = sizeof(device_extensions) / sizeof(device_extensions[0]);

static const uint32_t queue_families = IGNIS_QUEUE_FLAG_GRAPHICS
                                     | IGNIS_QUEUE_FLAG_TRANSFER
                                     | IGNIS_QUEUE_FLAG_PRESENT;


static uint32_t ignisFindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface, uint32_t* indices);

static uint8_t ignisQuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

static uint8_t ignisCheckDeviceExtensionSupport(VkPhysicalDevice device);


static uint8_t ignisPickPhysicalDevice(IgnisContext* context, IgnisDevice* device)
{
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(context->instance, &count, NULL);
    if (!count) return IGNIS_FAIL;

    // VkPhysicalDevice devices[count];
    VkPhysicalDevice* devices = ignisAlloc(sizeof(VkPhysicalDevice) * count);
    if (!devices) return IGNIS_FAIL;

    vkEnumeratePhysicalDevices(context->instance, &count, devices);

    device->physical = VK_NULL_HANDLE;
    for (uint32_t i = 0; i < count; ++i)
    {
        // queue families
        uint32_t family_indices[IGNIS_MAX_QUEUE_INDEX];
        uint32_t families_set = ignisFindQueueFamilies(devices[i], context->surface, family_indices);
        if (!(families_set & queue_families))
            continue;

        // swapchain support
        if (!ignisQuerySwapChainSupport(devices[i], context->surface))
            continue;

        // device extensions
        if (!ignisCheckDeviceExtensionSupport(devices[i]))
            continue;

        // suitable device found
        device->physical = devices[i];
        device->queue_families_set = families_set;
        memcpy(device->queue_family_indices, family_indices, sizeof(uint32_t) * IGNIS_MAX_QUEUE_INDEX);

        ignisPrintPhysicalDeviceInfo(devices[i]);
        break;
    }

    ignisFree(devices, sizeof(VkPhysicalDevice) * count);
    return device->physical != VK_NULL_HANDLE;
}


uint8_t ignisCreateDevice(IgnisContext* context)
{
    if (!ignisPickPhysicalDevice(context, &context->device))
    {
        return IGNIS_FAIL;
    }

    return IGNIS_OK;
}

void ignisDestroyDevice(IgnisContext* context)
{

}

void ignisPrintPhysicalDeviceInfo(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);

    VkPhysicalDeviceMemoryProperties memory;
    vkGetPhysicalDeviceMemoryProperties(device, &memory);

    MINIMAL_INFO("Physical device: %s", properties.deviceName);
    
    switch (properties.deviceType)
    {
        default:
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
            MINIMAL_INFO("GPU type is Unknown.");
            break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            MINIMAL_INFO("GPU type is Integrated.");
            break;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            MINIMAL_INFO("GPU type is Descrete.");
            break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            MINIMAL_INFO("GPU type is Virtual.");
            break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            MINIMAL_INFO("GPU type is CPU.");
            break;
    }

    MINIMAL_INFO("Driver Version: %d.%d.%d",
        VK_VERSION_MAJOR(properties.driverVersion),
        VK_VERSION_MINOR(properties.driverVersion),
        VK_VERSION_PATCH(properties.driverVersion));

    MINIMAL_INFO("Vulkan API Version: %d.%d.%d",
        VK_VERSION_MAJOR(properties.apiVersion),
        VK_VERSION_MINOR(properties.apiVersion),
        VK_VERSION_PATCH(properties.apiVersion));

    for (uint32_t i = 0; i < memory.memoryHeapCount; ++i)
    {
        float memory_size_gib = (((float)memory.memoryHeaps[i].size) / 1024.0f / 1024.0f / 1024.0f);

        if (memory.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
            MINIMAL_INFO("Local GPU memory: %.2f GiB", memory_size_gib);
        else
            MINIMAL_INFO("Shared System memory: %.2f GiB", memory_size_gib);
    }
}



uint32_t ignisFindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface, uint32_t* indices)
{
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, NULL);
    if (!count) return 0;

    VkQueueFamilyProperties* properties = ignisAlloc(sizeof(VkQueueFamilyProperties) * count);
    if (!properties) return 0;

    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, properties);

    uint32_t families_set = 0;
    uint8_t min_transfer_score = 255;
    for (uint32_t i = 0; i < count; ++i)
    {
        uint8_t current_transfer_score = 0;

        // graphics queue
        if (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices[IGNIS_QUEUE_GRAPHICS] = i;
            families_set |= IGNIS_QUEUE_FLAG_GRAPHICS;
            ++current_transfer_score;
        }

        // compute queue
        if (properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            indices[IGNIS_QUEUE_COMPUTE] = i;
            families_set |= IGNIS_QUEUE_FLAG_COMPUTE;
            ++current_transfer_score;
        }

        // transfer queue
        if (properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
        {
            // Take the index if it is the current lowest. This increases the
            // liklihood that it is a dedicated transfer queue.
            if (current_transfer_score <= min_transfer_score)
            {
                min_transfer_score = current_transfer_score;
                indices[IGNIS_QUEUE_TRANSFER] = i;
                families_set |= IGNIS_QUEUE_FLAG_TRANSFER;
            }
        }

        VkBool32 supported = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supported);
        if (supported)
        {
            indices[IGNIS_QUEUE_PRESENT] = i;
            families_set |= IGNIS_QUEUE_FLAG_PRESENT;
        }
    }

    ignisFree(properties, sizeof(VkQueueFamilyProperties) * count);
    return families_set;
}

uint8_t ignisQuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, NULL);

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, NULL);

    return format_count > 0 && present_mode_count > 0;
}

uint8_t ignisCheckDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t count = 0;
    vkEnumerateDeviceExtensionProperties(device, NULL, &count, NULL);
    if (!count || device_extension_count > count) return IGNIS_FAIL;

    VkExtensionProperties* properties = ignisAlloc(sizeof(VkExtensionProperties) * count);
    if (!properties) return IGNIS_FAIL;

    vkEnumerateDeviceExtensionProperties(device, NULL, &count, properties);

    uint8_t found = IGNIS_FAIL;
    for (size_t i = 0; i < device_extension_count; ++i)
    {
        found = IGNIS_FAIL;
        for (size_t j = 0; j < count; ++j)
        {
            if (strcmp(device_extensions[i], properties[j].extensionName) == 0)
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