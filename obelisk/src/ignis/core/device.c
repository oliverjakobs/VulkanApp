#include "../ignis_core.h"

#include "minimal/common.h"


// Requirements
static const char* const REQ_EXTENSIONS[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static const uint32_t REQ_EXTENSION_COUNT = sizeof(REQ_EXTENSIONS) / sizeof(REQ_EXTENSIONS[0]);

static const uint32_t REQ_QUEUE_FAMILIES = IGNIS_QUEUE_FLAG_GRAPHICS
                                        | IGNIS_QUEUE_FLAG_TRANSFER
                                        | IGNIS_QUEUE_FLAG_PRESENT;


static uint32_t ignisFindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface, uint32_t* indices);
static uint8_t ignisQuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
static uint8_t ignisCheckDeviceExtensionSupport(VkPhysicalDevice device);


static uint8_t ignisPickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, IgnisDevice* device)
{
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(instance, &count, NULL);
    if (!count) return IGNIS_FAIL;

    VkPhysicalDevice* devices = ignisAlloc(sizeof(VkPhysicalDevice) * count);
    if (!devices) return IGNIS_FAIL;

    vkEnumeratePhysicalDevices(instance, &count, devices);

    device->physical = VK_NULL_HANDLE;
    for (uint32_t i = 0; i < count; ++i)
    {
        // queue families
        uint32_t family_indices[IGNIS_QUEUE_MAX_ENUM];
        uint32_t families_set = ignisFindQueueFamilies(devices[i], surface, family_indices);
        if (!(families_set & REQ_QUEUE_FAMILIES))
            continue;

        // swapchain support
        if (!ignisQuerySwapChainSupport(devices[i], surface))
            continue;

        // device extensions
        if (!ignisCheckDeviceExtensionSupport(devices[i]))
            continue;

        // suitable device found
        device->physical = devices[i];
        device->queueFamiliesSet = families_set;
        memcpy(device->queueFamilyIndices, family_indices, sizeof(uint32_t) * IGNIS_QUEUE_MAX_ENUM);
        break;
    }

    ignisFree(devices, sizeof(VkPhysicalDevice) * count);
    return device->physical != VK_NULL_HANDLE;
}

static uint8_t ignisArrayCheckUnique(uint32_t arr[], uint32_t size)
{
    for (uint32_t i = 1; i < size; ++i)
        if (arr[0] == arr[i]) return IGNIS_FAIL;
    return IGNIS_OK;
}

uint8_t ignisCreateDevice(VkInstance instance, VkSurfaceKHR surface, IgnisDevice* device)
{
    if (!ignisPickPhysicalDevice(instance, surface, device))
    {
        MINIMAL_ERROR("failed to pick physical device");
        return IGNIS_FAIL;
    }

    uint32_t queue_count = 0;
    VkDeviceQueueCreateInfo queue_create_infos[IGNIS_QUEUE_MAX_ENUM] = { 0 };

    float queue_priority = 1.0f;
    for (uint32_t i = 0; i < IGNIS_QUEUE_MAX_ENUM; ++i)
    {
        if (!ignisArrayCheckUnique(device->queueFamilyIndices + i, IGNIS_QUEUE_MAX_ENUM - i))
            continue;

        queue_create_infos[queue_count++] = (VkDeviceQueueCreateInfo){
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = device->queueFamilyIndices[i],
            .queueCount = 1,
            .pQueuePriorities = &queue_priority,
        };
    }

    VkPhysicalDeviceFeatures device_features = { 0 };

    VkDeviceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = queue_create_infos,
        .queueCreateInfoCount = queue_count,
        .pEnabledFeatures = &device_features,
        .ppEnabledExtensionNames = REQ_EXTENSIONS,
        .enabledExtensionCount = REQ_EXTENSION_COUNT
    };

    VkResult result = vkCreateDevice(device->physical, &create_info, ignisGetAllocator(), &device->handle);
    if (result != VK_SUCCESS)
        return IGNIS_FAIL;

    /* get queues */
    for (size_t i = 0; i < IGNIS_QUEUE_MAX_ENUM; ++i)
        vkGetDeviceQueue(device->handle, device->queueFamilyIndices[i], 0, &device->queues[i]);

    return IGNIS_OK;
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
    if (!count || REQ_EXTENSION_COUNT > count) return IGNIS_FAIL;

    VkExtensionProperties* properties = ignisAlloc(sizeof(VkExtensionProperties) * count);
    if (!properties) return IGNIS_FAIL;

    vkEnumerateDeviceExtensionProperties(device, NULL, &count, properties);

    uint8_t found = IGNIS_FAIL;
    for (size_t i = 0; i < REQ_EXTENSION_COUNT; ++i)
    {
        found = IGNIS_FAIL;
        for (size_t j = 0; j < count; ++j)
        {
            if (strcmp(REQ_EXTENSIONS[i], properties[j].extensionName) == 0)
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

VkFormat ignisQueryDeviceDepthFormat(VkPhysicalDevice device)
{
    VkFormat candidates[] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };
    const uint32_t count = sizeof(candidates) / sizeof(candidates[0]);;

    VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    for (uint32_t i = 0; i < count; ++i)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(device, candidates[i], &props);

        if ((props.linearTilingFeatures & features) == features)
            return candidates[i];
        else if ((props.optimalTilingFeatures & features) == features)
            return candidates[i];
    }
    return VK_FORMAT_UNDEFINED;
}

int32_t ignisFindMemoryTypeIndex(VkPhysicalDevice device, uint32_t filter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memoryProps;
    vkGetPhysicalDeviceMemoryProperties(device, &memoryProps);

    for (uint32_t i = 0; i < memoryProps.memoryTypeCount; i++)
    {
        if ((filter & (1 << i)) && (memoryProps.memoryTypes[i].propertyFlags & properties) == properties)
            return (int32_t)i;
    }

    return -1;
}


void ignisPrintDeviceInfo(const IgnisDevice* device)
{
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device->physical, &properties);

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device->physical, &features);

    VkPhysicalDeviceMemoryProperties memory;
    vkGetPhysicalDeviceMemoryProperties(device->physical, &memory);

    MINIMAL_INFO("Physical device: %s", properties.deviceName);
    
    const char* typeDesc[] = {
        [VK_PHYSICAL_DEVICE_TYPE_OTHER] = "OTHER",
        [VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU] = "INTEGRATED GPU",
        [VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU] = "DISCRETE GPU",
        [VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU] = "VIRTUAL GPU",
        [VK_PHYSICAL_DEVICE_TYPE_CPU] = "CPU",
    };
    MINIMAL_INFO("  > Device Type: %s", typeDesc[properties.deviceType]);

    MINIMAL_INFO("  > Driver Version: %d.%d.%d",
        VK_VERSION_MAJOR(properties.driverVersion),
        VK_VERSION_MINOR(properties.driverVersion),
        VK_VERSION_PATCH(properties.driverVersion));

    MINIMAL_INFO("  > Vulkan API Version: %d.%d.%d",
        VK_VERSION_MAJOR(properties.apiVersion),
        VK_VERSION_MINOR(properties.apiVersion),
        VK_VERSION_PATCH(properties.apiVersion));

    MINIMAL_INFO("  > Memory:");
    for (uint32_t i = 0; i < memory.memoryHeapCount; ++i)
    {
        float memory_size_gib = (((float)memory.memoryHeaps[i].size) / 1024.0f / 1024.0f / 1024.0f);

        if (memory.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
            MINIMAL_INFO("    Local:  %.2f GiB", memory_size_gib);
        else
            MINIMAL_INFO("    Shared: %.2f GiB", memory_size_gib);
    }
}