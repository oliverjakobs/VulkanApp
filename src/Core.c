#include "Core.h"

#include <string.h>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data) {

    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        MINIMAL_ERROR("validation layer: %s", callback_data->pMessage);
    return VK_FALSE;
}

VkDebugUtilsMessengerCreateInfoEXT debugInfo = {
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

VkResult vkCreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* info,
    const VkAllocationCallbacks* allocator,
    VkDebugUtilsMessengerEXT* messenger) {

    PFN_vkVoidFunction func = vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func) return ((PFN_vkCreateDebugUtilsMessengerEXT)func)(instance, info, allocator, messenger);
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void vkDestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT messenger,
    const VkAllocationCallbacks* allocator) {

    PFN_vkVoidFunction func = vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func) ((PFN_vkDestroyDebugUtilsMessengerEXT)func)(instance, messenger, allocator);
}

const char* const validationLayers[] = {
    "VK_LAYER_KHRONOS_validation"
};

const uint32_t validationLayerCount = sizeof(validationLayers) / sizeof(validationLayers[0]);

typedef struct {
    uint32_t familiesSet;
    uint32_t graphicsFamily;
    uint32_t presentFamily;
} QueueFamilyIndices;

struct obeliskContext {
    VkInstance instance;
    VkSurfaceKHR surface;

    QueueFamilyIndices indices;

    VkPhysicalDevice physicalDevice;
    VkDevice device;

    VkCommandPool commandPool;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
};

static char** getRequiredExtensions(int debug, uint32_t* count) {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    if (!glfwExtensionCount) return NULL;

    char** extensions = malloc(sizeof(char*) * ((size_t)glfwExtensionCount + 1));
    if (!extensions) return NULL;

    memcpy(extensions, glfwExtensions, sizeof(char*) * glfwExtensionCount);

    if (debug) extensions[glfwExtensionCount++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

    *count = glfwExtensionCount;
    return extensions;
}

static int checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    if (!layerCount) return 0;

    VkLayerProperties* availableLayers = malloc(sizeof(VkLayerProperties) * layerCount);
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

    free(availableLayers);
    return layerFound;
}

int obeliskCreateInstance(obeliskContext* context, const char* app, const char* engine, int debug) {
    if (debug && !checkValidationLayerSupport()) {
        MINIMAL_ERROR("validation layers requested, but not available!");
        return MINIMAL_FAIL;
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
    char** extensions = getRequiredExtensions(debug, &extensionCount);

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
    }
    else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = NULL;
    }

    context->instance = VK_NULL_HANDLE;
    VkResult result = vkCreateInstance(&createInfo, NULL, &context->instance);

    free(extensions);

    if (result != VK_SUCCESS) {
        return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
}

typedef enum {
    OBELSIK_QUEUE_FAMILY_NONE = 0,
    OBELSIK_QUEUE_FAMILY_GRAPHICS = 1 << 0,
    OBELSIK_QUEUE_FAMILY_PRESENT = 1 << 1,
    OBELSIK_QUEUE_FAMILY_ALL = OBELSIK_QUEUE_FAMILY_GRAPHICS | OBELSIK_QUEUE_FAMILY_PRESENT
} QueueFamilyFlag;


int queueFamilyIndicesComplete(QueueFamilyIndices indices) {
    return indices.familiesSet & OBELSIK_QUEUE_FAMILY_GRAPHICS && indices.familiesSet & OBELSIK_QUEUE_FAMILY_PRESENT;
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
            indices.familiesSet |= OBELSIK_QUEUE_FAMILY_GRAPHICS;
        }

        VkBool32 supported = 0;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supported);
        if (supported) {
            indices.presentFamily = i;
            indices.familiesSet |= OBELSIK_QUEUE_FAMILY_PRESENT;
        }

        if (queueFamilyIndicesComplete(indices)) break;
    }

    free(properties);
    return indices;
}

const char* const deviceExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const uint32_t deviceExtensionCount = sizeof(deviceExtensions) / sizeof(deviceExtensions[0]);

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

int obeliskPickPhysicalDevice(obeliskContext* context) {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(context->instance, &device_count, NULL);
    if (!device_count) return MINIMAL_FAIL;

    VkPhysicalDevice* devices = malloc(sizeof(VkPhysicalDevice) * device_count);
    if (!devices) return MINIMAL_FAIL;

    vkEnumeratePhysicalDevices(context->instance, &device_count, devices);

    for (uint32_t i = 0; i < device_count; ++i) {
        QueueFamilyIndices indices = findQueueFamilies(devices[i], context->surface);
        if (queueFamilyIndicesComplete(indices) && isDeviceSuitable(devices[i], context->surface)) {
            context->physicalDevice = devices[i];
            context->indices = indices;
            break;
        }
    }

    free(devices);
    return context->physicalDevice != VK_NULL_HANDLE;
}

#define OBELSIK_MAX_QUEUE_COUNT 2

static int obeliskArrayCheckUnique(uint32_t arr[], uint32_t size) {
    for (uint32_t i = 1; i < size; ++i)
        if (arr[0] == arr[i]) return 0;
    return 1;
}

int obeliskCreateLogicalDevice(obeliskContext* context) {
    uint32_t queueCreateInfoCount = 0;
    VkDeviceQueueCreateInfo queueCreateInfos[OBELSIK_MAX_QUEUE_COUNT] = { 0 };

    uint32_t indexValues[] = { context->indices.graphicsFamily, context->indices.presentFamily };
    uint32_t indexValueCount = sizeof(indexValues) / sizeof(uint32_t);

    float queuePriority = 1.0f;
    for (uint32_t i = 0; i < indexValueCount; ++i) {
        if (!obeliskArrayCheckUnique(indexValues + i, indexValueCount - i))
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
    vkGetDeviceQueue(context->device, context->indices.graphicsFamily, 0, &context->graphicsQueue);
    vkGetDeviceQueue(context->device, context->indices.presentFamily, 0, &context->presentQueue);

    return MINIMAL_OK;
}

static obeliskContext _context = { 0 };
static VkDebugUtilsMessengerEXT _debugMessenger = VK_NULL_HANDLE;

int obeliskCreateContext(GLFWwindow* window, const char* app, int debug) {
    /* create instance */
    if (!obeliskCreateInstance(&_context, app, "obelisk", debug)) {
        MINIMAL_ERROR("Failed to create vulkan instance!");
        return MINIMAL_FAIL;
    }

    /* setup debug messenger */
    if (debug) {
        if (vkCreateDebugUtilsMessengerEXT(_context.instance, &debugInfo, NULL, &_debugMessenger) != VK_SUCCESS) {
            MINIMAL_ERROR("failed to set up debug messenger!");
            return MINIMAL_FAIL;
        }
    }

    /* create surface */
    if (glfwCreateWindowSurface(_context.instance, window, NULL, &_context.surface) != VK_SUCCESS) {
        MINIMAL_ERROR("failed to create window surface!");
        return MINIMAL_FAIL;
    }

    /* pick physical device */
    if (!obeliskPickPhysicalDevice(&_context)) {
        MINIMAL_ERROR("failed to find a suitable GPU!");
        return MINIMAL_FAIL;
    }

    /* create logical device */
    if (!obeliskCreateLogicalDevice(&_context)) {
        MINIMAL_ERROR("failed to create logical device!");
        return MINIMAL_FAIL;
    }

    /* create command pool */
    VkCommandPoolCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = _context.indices.graphicsFamily
    };

    if (vkCreateCommandPool(_context.device, &info, NULL, &_context.commandPool) != VK_SUCCESS) {
        MINIMAL_ERROR("failed to create command pool!");
        return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
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
uint32_t          obeliskGetQueueGraphicsFamilyIndex() { return _context.indices.graphicsFamily; }
uint32_t          obeliskGetQueuePresentFamilyIndex()  { return _context.indices.presentFamily; }

VkResult obeliskGetPhysicalDeviceSurfaceCapabilities(VkSurfaceCapabilitiesKHR* capabilities) {
    return vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_context.physicalDevice, _context.surface, capabilities);
}

void obeliskPrintInfo() {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(_context.physicalDevice, &properties);
    MINIMAL_INFO("Physical device: %s", properties.deviceName);
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