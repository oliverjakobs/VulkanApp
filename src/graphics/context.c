#include "context.h"

#include "../utils.h"
#include "../core/memory.h"

#include <string.h>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data) {

    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        OBELISK_ERROR("validation layer: %s", callback_data->pMessage);
    return VK_FALSE;
}

static VkDebugUtilsMessengerCreateInfoEXT debugInfo = {
    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
    .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
    .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
    .pfnUserCallback = debugCallback,
    .pUserData = NULL
};

static VkResult vkCreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* info,
    const VkAllocationCallbacks* allocator,
    VkDebugUtilsMessengerEXT* messenger) {

    PFN_vkVoidFunction func = vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func) return ((PFN_vkCreateDebugUtilsMessengerEXT)func)(instance, info, allocator, messenger);
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

static void vkDestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT messenger,
    const VkAllocationCallbacks* allocator) {

    PFN_vkVoidFunction func = vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func) ((PFN_vkDestroyDebugUtilsMessengerEXT)func)(instance, messenger, allocator);
}

static const char* const validationLayers[] = {
    "VK_LAYER_KHRONOS_validation"
};

static const uint32_t validationLayerCount = OBELISK_ARRAY_LEN(validationLayers);

typedef enum {
    OBELISK_QUEUE_GRAPHICS,
    OBELISK_QUEUE_PRESENT,
    OBELISK_QUEUE_COUNT
} ObeliskQueueFamilyIndex;

typedef enum {
    OBELISK_QUEUE_FLAG_NONE = 0,
    OBELISK_QUEUE_FLAG_GRAPHICS = 1 << 0,
    OBELISK_QUEUE_FLAG_PRESENT = 1 << 1,
    OBELISK_QUEUE_FLAG_ALL = OBELISK_QUEUE_FLAG_GRAPHICS | OBELISK_QUEUE_FLAG_PRESENT
} ObaliskQueueFamilyFlag;

typedef struct {
    VkInstance instance;
    VkSurfaceKHR surface;

    uint32_t queueFamiliesSet;
    uint32_t queueFamilyIndices[OBELISK_QUEUE_COUNT];

    VkPhysicalDevice physicalDevice;
    VkDevice device;

    VkCommandPool commandPool;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
} ObeliskContext;

static char** obeliskGetRequiredExtensions(int debug, uint32_t* count) {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    if (!glfwExtensionCount) return NULL;

    if (debug) glfwExtensionCount++;

    char** extensions = obeliskMemDup(glfwExtensions, sizeof(char*) * ((size_t)glfwExtensionCount));

    if (debug) extensions[glfwExtensionCount - 1] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

    *count = glfwExtensionCount;
    return extensions;
}

static int obeliskCheckValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    if (!layerCount) return 0;

    VkLayerProperties* availableLayers = obeliskAllocate(sizeof(VkLayerProperties) * layerCount);
    if (!availableLayers) return 0;

    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    int layerFound = 0;
    for (size_t i = 0; i < validationLayerCount; ++i) {
        layerFound = 0;
        const char* name = validationLayers[i];

        for (size_t available = 0; available < layerCount; ++available) {
            if (strcmp(name, availableLayers[available].layerName) == 0) {
                layerFound = 1;
                break;
            }
        }

        if (!layerFound) break;
    }

    obeliskFree(availableLayers);
    return layerFound;
}

static int obeliskCreateInstance(ObeliskContext* context, const char* app, const char* engine, int debug) {
    if (debug && !obeliskCheckValidationLayerSupport()) {
        OBELISK_ERROR("validation layers requested, but not available!");
        return OBELISK_FAIL;
    }

    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = app,
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = engine,
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0
    };

    uint32_t extensionCount = 0;
    char** extensions = obeliskGetRequiredExtensions(debug, &extensionCount);

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .ppEnabledExtensionNames = extensions,
        .enabledExtensionCount = extensionCount
    };

    if (debug) {
        createInfo.enabledLayerCount = validationLayerCount;
        createInfo.ppEnabledLayerNames = validationLayers;
        createInfo.pNext = &debugInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = NULL;
    }

    context->instance = VK_NULL_HANDLE;
    VkResult result = vkCreateInstance(&createInfo, NULL, &context->instance);

    obeliskFree(extensions);

    if (result != VK_SUCCESS) {
        return OBELISK_FAIL;
    }

    return OBELISK_OK;
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

        VkBool32 supported = 0;
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

static const char* const deviceExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static const uint32_t deviceExtensionCount = OBELISK_ARRAY_LEN(deviceExtensions);

static int obaliskCheckDeviceExtensionSupport(VkPhysicalDevice device) {
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

static int obaliskQuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, NULL);

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, NULL);

    return formatCount > 0 && presentModeCount > 0;
}

static int obeliskIsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
    if (!obaliskCheckDeviceExtensionSupport(device)) return 0;
    return obaliskQuerySwapChainSupport(device, surface);
}

static int obeliskPickPhysicalDevice(ObeliskContext* context) {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(context->instance, &device_count, NULL);
    if (!device_count) return OBELISK_FAIL;

    VkPhysicalDevice* devices = obeliskAllocate(sizeof(VkPhysicalDevice) * device_count);
    if (!devices) return OBELISK_FAIL;

    vkEnumeratePhysicalDevices(context->instance, &device_count, devices);

    for (uint32_t i = 0; i < device_count; ++i) {
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

static int obeliskCreateLogicalDevice(ObeliskContext* context) {
    uint32_t queueCreateInfoCount = 0;
    VkDeviceQueueCreateInfo queueCreateInfos[OBELISK_QUEUE_COUNT] = { 0 };

    uint32_t indexValues[] = { 
        context->queueFamilyIndices[OBELISK_QUEUE_GRAPHICS],
        context->queueFamilyIndices[OBELISK_QUEUE_PRESENT]
    };

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

    if (vkCreateDevice(context->physicalDevice, &createInfo, NULL, &context->device) != VK_SUCCESS)
        return OBELISK_FAIL;

    /* get queues */
    vkGetDeviceQueue(context->device, context->queueFamilyIndices[OBELISK_QUEUE_GRAPHICS], 0, &context->graphicsQueue);
    vkGetDeviceQueue(context->device, context->queueFamilyIndices[OBELISK_QUEUE_PRESENT], 0, &context->presentQueue);

    return OBELISK_OK;
}

static ObeliskContext _context = { 0 };
static VkDebugUtilsMessengerEXT _debugMessenger = VK_NULL_HANDLE;

int obeliskCreateContext(GLFWwindow* window, const char* app, int debug) {
    /* create instance */
    if (!obeliskCreateInstance(&_context, app, "obelisk", debug)) {
        OBELISK_ERROR("Failed to create vulkan instance!");
        return OBELISK_FAIL;
    }

    /* setup debug messenger */
    if (debug) {
        if (vkCreateDebugUtilsMessengerEXT(_context.instance, &debugInfo, NULL, &_debugMessenger) != VK_SUCCESS) {
            OBELISK_ERROR("failed to set up debug messenger!");
            return OBELISK_FAIL;
        }
    }

    /* create surface */
    if (glfwCreateWindowSurface(_context.instance, window, NULL, &_context.surface) != VK_SUCCESS) {
        OBELISK_ERROR("failed to create window surface!");
        return OBELISK_FAIL;
    }

    /* pick physical device */
    if (!obeliskPickPhysicalDevice(&_context)) {
        OBELISK_ERROR("failed to find a suitable GPU!");
        return OBELISK_FAIL;
    }

    /* create logical device */
    if (!obeliskCreateLogicalDevice(&_context)) {
        OBELISK_ERROR("failed to create logical device!");
        return OBELISK_FAIL;
    }

    /* create command pool */
    VkCommandPoolCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = _context.queueFamilyIndices[OBELISK_QUEUE_GRAPHICS]
    };

    if (vkCreateCommandPool(_context.device, &info, NULL, &_context.commandPool) != VK_SUCCESS) {
        OBELISK_ERROR("failed to create command pool!");
        return OBELISK_FAIL;
    }

    return OBELISK_OK;
}

void obeliskDestroyContext() {
    /* destroy command pool */
    vkDestroyCommandPool(_context.device, _context.commandPool, NULL);

    /* destroy device */
    vkDestroyDevice(_context.device, NULL);

    /* destroy debug messenger */
    if (_debugMessenger != VK_NULL_HANDLE)
        vkDestroyDebugUtilsMessengerEXT(_context.instance, _debugMessenger, NULL);

    /* destroy surface and instance */
    vkDestroySurfaceKHR(_context.instance, _context.surface, NULL);
    vkDestroyInstance(_context.instance, NULL);
}

VkDevice          obeliskGetDevice()         { return _context.device; }
VkPhysicalDevice  obeliskGetPhysicalDevice() { return _context.physicalDevice; }
VkSurfaceKHR      obeliskGetSurface()        { return _context.surface; }
VkQueue           obeliskGetGraphicsQueue()  { return _context.graphicsQueue; }
VkQueue           obeliskGetPresentQueue()   { return _context.presentQueue; }
uint32_t          obeliskGetQueueGraphicsFamilyIndex() { return _context.queueFamilyIndices[OBELISK_QUEUE_GRAPHICS]; }
uint32_t          obeliskGetQueuePresentFamilyIndex()  { return _context.queueFamilyIndices[OBELISK_QUEUE_PRESENT]; }

VkResult obeliskGetPhysicalDeviceSurfaceCapabilities(VkSurfaceCapabilitiesKHR* capabilities) {
    return vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_context.physicalDevice, _context.surface, capabilities);
}

VkFormat obeliskGetPhysicalDeviceFormat(const VkFormat* candidates, uint32_t count, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (uint32_t i = 0; i < count; ++i) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(_context.physicalDevice, candidates[i], &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            return candidates[i];
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            return candidates[i];
    }
    return VK_FORMAT_UNDEFINED;
}

uint32_t obeliskFindMemoryTypeIndex(uint32_t filter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memoryProps;
    vkGetPhysicalDeviceMemoryProperties(obeliskGetPhysicalDevice(), &memoryProps);

    for (uint32_t i = 0; i < memoryProps.memoryTypeCount; i++) {
        if ((filter & (1 << i)) && (memoryProps.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }

    OBELISK_ERROR("failed to find suitable memory type!");
    return 0;
}

void obeliskPrintInfo() {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(_context.physicalDevice, &properties);
    OBELISK_INFO("Physical device: %s", properties.deviceName);
}

VkResult obeliskAllocateCommandBuffers(VkCommandBuffer* buffers, VkCommandBufferLevel level, uint32_t count) {
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = level,
        .commandPool = _context.commandPool,
        .commandBufferCount = count
    };

    return vkAllocateCommandBuffers(_context.device, &allocInfo, buffers);
}

void obeliskFreeCommandBuffers(const VkCommandBuffer* buffers, uint32_t count) {
    vkFreeCommandBuffers(_context.device, _context.commandPool, count, buffers);
}