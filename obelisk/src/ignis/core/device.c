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

static uint8_t ignisArrayCheckUnique(uint32_t arr[], uint32_t size)
{
    for (uint32_t i = 1; i < size; ++i)
        if (arr[0] == arr[i]) return IGNIS_FAIL;
    return IGNIS_OK;
}

uint8_t ignisCreateDevice(VkInstance instance, VkSurfaceKHR surface, IgnisDevice* device)
{
    // pick physical device
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(instance, &count, NULL);
    if (!count) return IGNIS_FAIL;

    VkPhysicalDevice* devices = ignisAlloc(sizeof(VkPhysicalDevice) * count);
    if (!devices) return IGNIS_FAIL;

    vkEnumeratePhysicalDevices(instance, &count, devices);

    device->physical = VK_NULL_HANDLE;
    for (uint32_t i = 0; i < count; ++i)
    {
        // skip device if required queue families are not supported
        uint32_t familyIndices[IGNIS_QUEUE_MAX_ENUM];
        uint32_t familiesSet = ignisFindQueueFamilies(devices[i], surface, familyIndices);
        if (!(familiesSet & REQ_QUEUE_FAMILIES))
            continue;

        // skip device if required swapchain is not support
        if (!ignisQuerySwapChainSupport(devices[i], surface))
            continue;

        // skip device if required extensions are not supported
        if (!ignisCheckDeviceExtensionSupport(devices[i]))
            continue;

        // suitable device found
        device->physical = devices[i];
        device->queueFamiliesSet = familiesSet;
        memcpy(device->queueFamilyIndices, familyIndices, sizeof(uint32_t) * IGNIS_QUEUE_MAX_ENUM);
        break;
    }

    ignisFree(devices, sizeof(VkPhysicalDevice) * count);

    if (device->physical == VK_NULL_HANDLE)
    {
        MINIMAL_ERROR("failed to pick physical device");
        return IGNIS_FAIL;
    }

    /* create logical device */
    uint32_t queueCount = 0;
    VkDeviceQueueCreateInfo queueCreateInfos[IGNIS_QUEUE_MAX_ENUM] = { 0 };

    float priority = 1.0f;
    for (uint32_t i = 0; i < IGNIS_QUEUE_MAX_ENUM; ++i)
    {
        if (!ignisArrayCheckUnique(device->queueFamilyIndices + i, IGNIS_QUEUE_MAX_ENUM - i))
            continue;

        queueCreateInfos[queueCount++] = (VkDeviceQueueCreateInfo){
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = device->queueFamilyIndices[i],
            .queueCount = 1,
            .pQueuePriorities = &priority,
        };
    }

    VkPhysicalDeviceFeatures deviceFeatures = { 0 };

    VkDeviceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = queueCreateInfos,
        .queueCreateInfoCount = queueCount,
        .pEnabledFeatures = &deviceFeatures,
        .ppEnabledExtensionNames = REQ_EXTENSIONS,
        .enabledExtensionCount = REQ_EXTENSION_COUNT
    };

    VkResult result = vkCreateDevice(device->physical, &createInfo, ignisGetAllocator(), &device->handle);
    if (result != VK_SUCCESS)
        return IGNIS_FAIL;

    /* get queues */
    for (size_t i = 0; i < IGNIS_QUEUE_MAX_ENUM; ++i)
        vkGetDeviceQueue(device->handle, device->queueFamilyIndices[i], 0, &device->queues[i]);

    /* create command pool */
    VkCommandPoolCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = device->queueFamilyIndices[IGNIS_QUEUE_GRAPHICS]
    };

    if (vkCreateCommandPool(device->handle, &info, ignisGetAllocator(), &device->commandPool) != VK_SUCCESS)
    {
        MINIMAL_ERROR("failed to create device command pool!");
        return IGNIS_FAIL;
    }

    return IGNIS_OK;
}

void ignisDestroyDevice(IgnisDevice* device)
{
    vkDestroyCommandPool(device->handle, device->commandPool, ignisGetAllocator());
    vkDestroyDevice(device->handle, ignisGetAllocator());
}

uint32_t ignisFindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface, uint32_t* indices)
{
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, NULL);
    if (!count) return 0;

    VkQueueFamilyProperties* properties = ignisAlloc(sizeof(VkQueueFamilyProperties) * count);
    if (!properties) return 0;

    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, properties);

    uint32_t familiesSet = 0;
    uint8_t minTransferScore = -1;
    for (uint32_t i = 0; i < count; ++i)
    {
        uint8_t currentTransferScore = 0;

        // graphics queue
        if (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices[IGNIS_QUEUE_GRAPHICS] = i;
            familiesSet |= IGNIS_QUEUE_FLAG_GRAPHICS;
            ++currentTransferScore;
        }

        // compute queue
        if (properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            indices[IGNIS_QUEUE_COMPUTE] = i;
            familiesSet |= IGNIS_QUEUE_FLAG_COMPUTE;
            ++currentTransferScore;
        }

        // transfer queue
        if (properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
        {
            // Take the index if it is the current lowest. This increases the
            // liklihood that it is a dedicated transfer queue.
            if (currentTransferScore <= minTransferScore)
            {
                minTransferScore = currentTransferScore;
                indices[IGNIS_QUEUE_TRANSFER] = i;
                familiesSet |= IGNIS_QUEUE_FLAG_TRANSFER;
            }
        }

        VkBool32 supported = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supported);
        if (supported)
        {
            indices[IGNIS_QUEUE_PRESENT] = i;
            familiesSet |= IGNIS_QUEUE_FLAG_PRESENT;
        }
    }

    ignisFree(properties, sizeof(VkQueueFamilyProperties) * count);
    return familiesSet;
}

uint8_t ignisQuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, NULL);

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, NULL);

    return formatCount > 0 && presentModeCount > 0;
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

void ignisPrintPhysicalDeviceInfo(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);

    VkPhysicalDeviceMemoryProperties memory;
    vkGetPhysicalDeviceMemoryProperties(device, &memory);

    MINIMAL_INFO("Physical device: %s", properties.deviceName);
    
    const char* typeDesc[] = {
        [VK_PHYSICAL_DEVICE_TYPE_OTHER]          = "OTHER",
        [VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU] = "INTEGRATED GPU",
        [VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU]   = "DISCRETE GPU",
        [VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU]    = "VIRTUAL GPU",
        [VK_PHYSICAL_DEVICE_TYPE_CPU]            = "CPU",
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




VkDeviceMemory ignisAllocateDeviceMemory(const IgnisDevice* device, VkMemoryRequirements requirements, VkMemoryPropertyFlags properties, const VkAllocationCallbacks* allocator)
{
    VkPhysicalDeviceMemoryProperties memoryProps;
    vkGetPhysicalDeviceMemoryProperties(device->physical, &memoryProps);

    uint32_t memoryTypeIndex = memoryProps.memoryTypeCount;
    for (uint32_t i = 0; i < memoryProps.memoryTypeCount; ++i)
    {
        if ((requirements.memoryTypeBits & (1 << i)) && (memoryProps.memoryTypes[i].propertyFlags & properties) == properties)
        {
            memoryTypeIndex = i;
            break;
        }
    }

    if (memoryTypeIndex == memoryProps.memoryTypeCount)
    {
        MINIMAL_ERROR("failed to find suitable memory type!");
        return VK_NULL_HANDLE;
    }

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = requirements.size,
        .memoryTypeIndex = memoryTypeIndex
    };

    VkDeviceMemory memory = VK_NULL_HANDLE;
    if (vkAllocateMemory(device->handle, &allocInfo, allocator, &memory) != VK_SUCCESS)
        MINIMAL_ERROR("failed to find allocate device memory!");

    return memory;
}


