#include "device.h"

#include "Minimal/Utils.h"
#include <string.h>

const char* const deviceExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const uint32_t deviceExtensionCount = sizeof(deviceExtensions) / sizeof(deviceExtensions[0]);

typedef enum {
    IGNIS_QUEUE_FAMILY_NONE = 0,
    IGNIS_QUEUE_FAMILY_GRAPHICS = 1 << 0,
    IGNIS_QUEUE_FAMILY_PRESENT = 1 << 1,
} QueueFamilyFlag;

int QueueFamilyIndicesComplete(QueueFamilyIndices indices) {
    return indices.familiesSet & IGNIS_QUEUE_FAMILY_GRAPHICS 
        && indices.familiesSet & IGNIS_QUEUE_FAMILY_PRESENT;
}

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices = { 0 };

    uint32_t propertyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &propertyCount, NULL);
    if (!propertyCount) return indices;

    VkQueueFamilyProperties* properties = malloc(sizeof(VkQueueFamilyProperties) * propertyCount);
    if (!properties) return indices;

    vkGetPhysicalDeviceQueueFamilyProperties(device, &propertyCount, properties);

    for (uint32_t i = 0; i < propertyCount; ++i) {
        if (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
            indices.familiesSet |= IGNIS_QUEUE_FAMILY_GRAPHICS;
        }

        VkBool32 supported = 0;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supported);
        if (supported) {
            indices.presentFamily = i;
            indices.familiesSet |= IGNIS_QUEUE_FAMILY_PRESENT;
        }

        if (QueueFamilyIndicesComplete(indices))
            break;
    }

    return indices;
}

static int CheckDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);
    if (!extensionCount) return 0;

    VkExtensionProperties* availableExtensions = malloc(sizeof(VkExtensionProperties) * extensionCount);
    if (!availableExtensions) return 0;

    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, availableExtensions);

    int extensionFound = 0;
    for (size_t i = 0; i < deviceExtensionCount; ++i)
    {
        extensionFound = 0;
        const char* name = deviceExtensions[i];

        for (size_t available = 0; available < extensionCount; ++available) {
            if (strcmp(name, availableExtensions[available].extensionName) == 0) {
                extensionFound = 1;
                break;
            }
        }

        if (!extensionFound) break;
    }

    free(availableExtensions);
    return extensionFound;
}

int IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
    if (!CheckDeviceExtensionSupport(device)) return 0;
    return querySwapChainSupport(device, surface);
}

VkPhysicalDevice PickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, QueueFamilyIndices* pIndices) {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, NULL);
    if (!device_count) return VK_NULL_HANDLE;

    VkPhysicalDevice* devices = malloc(sizeof(VkPhysicalDevice) * device_count);
    if (!devices) return VK_NULL_HANDLE;

    vkEnumeratePhysicalDevices(instance, &device_count, devices);

    VkPhysicalDevice device = VK_NULL_HANDLE;
    for (uint32_t i = 0; i < device_count; ++i) {
        QueueFamilyIndices indices = FindQueueFamilies(devices[i], surface);
        if (QueueFamilyIndicesComplete(indices) && IsDeviceSuitable(devices[i], surface)) {
            device = devices[i];
            *pIndices = indices;
            break;
        }
    }

    free(devices);
    return device;
}

#define IGNIS_MAX_QUEUE_COUNT 2

static int ArrayCheckUnique(uint32_t arr[], uint32_t size) {
    for (uint32_t i = 1; i < size; ++i) {
        if (arr[0] == arr[i]) return 0;
    }
    return 1;
}

VkDevice CreateLogicalDevice(VkPhysicalDevice physicalDevice, QueueFamilyIndices indices) {
    uint32_t queueCreateInfoCount = 0;
    VkDeviceQueueCreateInfo queueCreateInfos[IGNIS_MAX_QUEUE_COUNT] = { 0 };

    uint32_t indexValues[] = { indices.graphicsFamily, indices.presentFamily };
    uint32_t indexValueCount = sizeof(indexValues) / sizeof(indexValues[0]);

    float queuePriority = 1.0f;
    for (uint32_t i = 0; i < indexValueCount; ++i) {
        if (!ArrayCheckUnique(indexValues + i, indexValueCount - i))
            continue;

        VkDeviceQueueCreateInfo queueCreateInfo = { 0 };
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = indexValues[i];
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos[queueCreateInfoCount++] = queueCreateInfo;
    }

    VkPhysicalDeviceFeatures deviceFeatures = { 0 };

    VkDeviceCreateInfo createInfo = { 0 };
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.pQueueCreateInfos = queueCreateInfos;
    createInfo.queueCreateInfoCount = queueCreateInfoCount;

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.ppEnabledExtensionNames = deviceExtensions;
    createInfo.enabledExtensionCount = deviceExtensionCount;

    VkDevice device;
    if (vkCreateDevice(physicalDevice, &createInfo, NULL, &device) != VK_SUCCESS)
        return VK_NULL_HANDLE;

    return device;
}