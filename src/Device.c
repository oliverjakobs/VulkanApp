#include "device.h"

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

int queueFamilyIndicesComplete(QueueFamilyIndices indices) {
    return indices.familiesSet & IGNIS_QUEUE_FAMILY_GRAPHICS && indices.familiesSet & IGNIS_QUEUE_FAMILY_PRESENT;
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
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

        if (queueFamilyIndicesComplete(indices)) break;
    }

    free(properties);
    return indices;
}

static int checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);
    if (!extensionCount) return 0;

    VkExtensionProperties* availableExtensions = malloc(sizeof(VkExtensionProperties) * extensionCount);
    if (!availableExtensions) return 0;

    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, availableExtensions);

    for (size_t i = 0; i < deviceExtensionCount; ++i) {
        const char* name = deviceExtensions[i];
        for (size_t available = 0; available < extensionCount; ++available) {
            if (strcmp(name, availableExtensions[available].extensionName) == 0) {
                free(availableExtensions);
                return 1;
            }
        }
    }

    free(availableExtensions);
    return 0;
}

static int querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, NULL);

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, NULL);

    return formatCount > 0 && presentModeCount > 0;
}

static int isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
    if (!checkDeviceExtensionSupport(device)) return 0;
    return querySwapChainSupport(device, surface);
}

int pickPhysicalDevice(VulkanContext* context, QueueFamilyIndices* indices) {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(context->instance, &device_count, NULL);
    if (!device_count) return MINIMAL_FAIL;

    VkPhysicalDevice* devices = malloc(sizeof(VkPhysicalDevice) * device_count);
    if (!devices) return MINIMAL_FAIL;

    vkEnumeratePhysicalDevices(context->instance, &device_count, devices);

    for (uint32_t i = 0; i < device_count; ++i) {
        QueueFamilyIndices ind = findQueueFamilies(devices[i], context->surface);
        if (queueFamilyIndicesComplete(ind) && isDeviceSuitable(devices[i], context->surface)) {
            context->physicalDevice = devices[i];
            *indices = ind;
            break;
        }
    }

    free(devices);
    return context->physicalDevice != VK_NULL_HANDLE;
}

#define IGNIS_MAX_QUEUE_COUNT 2

static int arrayCheckUnique(uint32_t arr[], uint32_t size) {
    for (uint32_t i = 1; i < size; ++i) {
        if (arr[0] == arr[i]) return 0;
    }
    return 1;
}

int createLogicalDevice(VulkanContext* context, QueueFamilyIndices indices) {
    uint32_t queueCreateInfoCount = 0;
    VkDeviceQueueCreateInfo queueCreateInfos[IGNIS_MAX_QUEUE_COUNT] = { 0 };

    uint32_t indexValues[] = { indices.graphicsFamily, indices.presentFamily };
    uint32_t indexValueCount = sizeof(indexValues) / sizeof(indexValues[0]);

    float queuePriority = 1.0f;
    for (uint32_t i = 0; i < indexValueCount; ++i) {
        if (!arrayCheckUnique(indexValues + i, indexValueCount - i))
            continue;

        queueCreateInfos[queueCreateInfoCount++] = (VkDeviceQueueCreateInfo){
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = indexValues[i],
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

    if (vkCreateDevice(context->physicalDevice, &createInfo, NULL, &context->device) != VK_SUCCESS)
        return MINIMAL_FAIL;

    /* get queues */
    vkGetDeviceQueue(context->device, indices.graphicsFamily, 0, &context->graphicsQueue);
    vkGetDeviceQueue(context->device, indices.presentFamily, 0, &context->presentQueue);

    return MINIMAL_OK;
}