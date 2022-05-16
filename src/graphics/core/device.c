#include "core.h"

#include "../../utility/memory.h"

static const char* const deviceExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static const uint32_t deviceExtensionCount = OBELISK_ARRAY_LEN(deviceExtensions);

static int obeliskCheckDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);
    if (!extensionCount) return 0;

    VkExtensionProperties* availableExtensions = obeliskAllocate(sizeof(VkExtensionProperties) * extensionCount);
    if (!availableExtensions) return 0;

    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, availableExtensions);

    for (size_t i = 0; i < deviceExtensionCount; ++i) {
        const char* name = deviceExtensions[i];
        for (size_t available = 0; available < extensionCount; ++available) {
            if (strcmp(name, availableExtensions[available].extensionName) == 0) {
                obeliskFree(availableExtensions);
                return 1;
            }
        }
    }

    obeliskFree(availableExtensions);
    return 0;
}

static int obeliskQuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, NULL);

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, NULL);

    return formatCount > 0 && presentModeCount > 0;
}

static int obeliskIsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
    if (!obeliskCheckDeviceExtensionSupport(device)) return 0;
    return obeliskQuerySwapChainSupport(device, surface);
}

static uint32_t obeliskFindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface, uint32_t* indices) {
    uint32_t propertyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &propertyCount, NULL);
    if (!propertyCount) return 0;

    VkQueueFamilyProperties* properties = obeliskAllocate(sizeof(VkQueueFamilyProperties) * propertyCount);
    if (!properties) return 0;

    vkGetPhysicalDeviceQueueFamilyProperties(device, &propertyCount, properties);

    uint32_t familiesSet = 0;
    for (uint32_t i = 0; i < propertyCount; ++i) {
        if (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices[OBELISK_QUEUE_GRAPHICS] = i;
            familiesSet |= OBELISK_QUEUE_FLAG_GRAPHICS;
        }

        VkBool32 supported = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supported);
        if (supported) {
            indices[OBELISK_QUEUE_PRESENT] = i;
            familiesSet |= OBELISK_QUEUE_FLAG_PRESENT;
        }

        if (familiesSet & OBELISK_QUEUE_FLAG_ALL) break;
    }

    obeliskFree(properties);
    return familiesSet;
}

int obeliskPickPhysicalDevice(ObeliskContext* context) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(context->instance, &deviceCount, NULL);
    if (!deviceCount) return OBELISK_FAIL;

    VkPhysicalDevice* devices = obeliskAllocate(sizeof(VkPhysicalDevice) * deviceCount);
    if (!devices) return OBELISK_FAIL;

    vkEnumeratePhysicalDevices(context->instance, &deviceCount, devices);

    context->physicalDevice = VK_NULL_HANDLE;
    for (uint32_t i = 0; i < deviceCount; ++i) {
        uint32_t familiesSet = obeliskFindQueueFamilies(devices[i], context->surface, context->queueFamilyIndices);
        if (familiesSet & OBELISK_QUEUE_FLAG_ALL && obeliskIsDeviceSuitable(devices[i], context->surface)) {
            context->physicalDevice = devices[i];
            context->queueFamiliesSet = familiesSet;
            break;
        }
    }

    obeliskFree(devices);
    return context->physicalDevice != VK_NULL_HANDLE;
}

static int obeliskArrayCheckUnique(uint32_t arr[], uint32_t size) {
    for (uint32_t i = 1; i < size; ++i)
        if (arr[0] == arr[i]) return 0;
    return 1;
}

int obeliskCreateLogicalDevice(ObeliskContext* context) {
    uint32_t queueCreateInfoCount = 0;
    VkDeviceQueueCreateInfo queueCreateInfos[OBELISK_QUEUE_COUNT] = { 0 };

    float queuePriority = 1.0f;
    for (uint32_t i = 0; i < OBELISK_QUEUE_COUNT; ++i) {
        if (!obeliskArrayCheckUnique(context->queueFamilyIndices + i, OBELISK_QUEUE_COUNT - i))
            continue;

        queueCreateInfos[queueCreateInfoCount++] = (VkDeviceQueueCreateInfo){
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = context->queueFamilyIndices[i],
            .queueCount = 1,
            .pQueuePriorities = &queuePriority,
        };
    }

    VkPhysicalDeviceFeatures deviceFeatures = { 0 };

    VkDeviceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = queueCreateInfos,
        .queueCreateInfoCount = queueCreateInfoCount,
        .pEnabledFeatures = &deviceFeatures,
        .ppEnabledExtensionNames = deviceExtensions,
        .enabledExtensionCount = deviceExtensionCount
    };

    if (vkCreateDevice(context->physicalDevice, &createInfo, context->allocator, &context->device) != VK_SUCCESS)
        return OBELISK_FAIL;

    /* get queues */
    for (size_t i = 0; i < OBELISK_QUEUE_COUNT; ++i) {
        vkGetDeviceQueue(context->device, context->queueFamilyIndices[i], 0, &context->queues[i]);
    }

    return OBELISK_OK;
}

VkResult obeliskGetPhysicalDeviceSurfaceCapabilities(VkSurfaceCapabilitiesKHR* capabilities) {
    return vkGetPhysicalDeviceSurfaceCapabilitiesKHR(obeliskGetPhysicalDevice(), obeliskGetSurface(), capabilities);
}

VkFormat obeliskGetPhysicalDeviceFormat(
        const VkFormat* candidates, 
        uint32_t count, 
        VkImageTiling tiling, 
        VkFormatFeatureFlags features) {
    for (uint32_t i = 0; i < count; ++i) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(obeliskGetPhysicalDevice(), candidates[i], &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            return candidates[i];
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            return candidates[i];
    }
    return VK_FORMAT_UNDEFINED;
}

uint32_t obeliskFindPhysicalDeviceMemoryTypeIndex(uint32_t filter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memoryProps;
    vkGetPhysicalDeviceMemoryProperties(obeliskGetPhysicalDevice(), &memoryProps);

    for (uint32_t i = 0; i < memoryProps.memoryTypeCount; i++) {
        if ((filter & (1 << i)) && (memoryProps.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }

    OBELISK_ERROR("failed to find suitable memory type!");
    return 0;
}

void obeliskPrintDeviceInfo() {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(obeliskGetPhysicalDevice(), &properties);

    OBELISK_INFO("Physical device: %s", properties.deviceName);
    OBELISK_INFO("QueueFamilies:");
    OBELISK_INFO("Graphics | Present");
    OBELISK_INFO("       %d |       %d",
        obeliskGetQueueFamilyIndices()[OBELISK_QUEUE_GRAPHICS],
        obeliskGetQueueFamilyIndices()[OBELISK_QUEUE_PRESENT]);

    OBELISK_INFO("Driver Version: %d.%d.%d",
        VK_VERSION_MAJOR(properties.driverVersion),
        VK_VERSION_MINOR(properties.driverVersion),
        VK_VERSION_PATCH(properties.driverVersion));

    OBELISK_INFO("Vulkan API Version: %d.%d.%d",
        VK_VERSION_MAJOR(properties.apiVersion),
        VK_VERSION_MINOR(properties.apiVersion),
        VK_VERSION_PATCH(properties.apiVersion));
}