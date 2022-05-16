#include "core.h"

#include "../../utility/memory.h"

/* ---------------------------------------------| debug |--------------------------------------------- */
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data) {

    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        OBELISK_ERROR("[Vulkan] %s", callback_data->pMessage);
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

static int obeliskCheckValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    if (!layerCount) return 0;

    VkLayerProperties* availableLayers = obeliskAllocate(sizeof(VkLayerProperties) * layerCount);
    if (!availableLayers) return 0;

    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    int found = 0;
    for (size_t i = 0; i < validationLayerCount; ++i) {
        found = 0;
        for (size_t available = 0; available < layerCount; ++available) {
            if (strcmp(validationLayers[i], availableLayers[available].layerName) == 0) {
                found = 1;
                break;
            }
        }

        if (!found) break;
    }

    obeliskFree(availableLayers);
    return found;
}

/* ---------------------------------------------| context |------------------------------------------- */
static char** obeliskGetRequiredExtensions(int debug, uint32_t* count) {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    if (!glfwExtensionCount) return NULL;

    char** extensions = obeliskMemDup(glfwExtensions, sizeof(char*) * ((size_t)glfwExtensionCount + 1));

    if (debug) extensions[glfwExtensionCount++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

    *count = glfwExtensionCount;
    return extensions;
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
        .engineVersion = VK_MAKE_VERSION(OBELISK_VERSION_MAJOR, OBELISK_VERSION_MINOR, OBELISK_VERSION_REVISION),
        .apiVersion = VK_API_VERSION_1_0
    };

    uint32_t extensionCount = 0;
    char** extensions = obeliskGetRequiredExtensions(debug, &extensionCount);

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .ppEnabledExtensionNames = extensions,
        .enabledExtensionCount = extensionCount,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = NULL,
        .pNext = NULL
    };

    if (debug) {
        createInfo.enabledLayerCount = validationLayerCount;
        createInfo.ppEnabledLayerNames = validationLayers;
        createInfo.pNext = &debugInfo;

        /* print all required extensions */
        OBELISK_INFO("Required extensions:");
        for (size_t i = 0; i < extensionCount; ++i) {
            OBELISK_INFO(" - %s", extensions[i]);
        }
    }

    context->instance = VK_NULL_HANDLE;
    VkResult result = vkCreateInstance(&createInfo, context->allocator, &context->instance);

    obeliskFree(extensions);

    if (result != VK_SUCCESS) {
        return OBELISK_FAIL;
    }

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
        if (vkCreateDebugUtilsMessengerEXT(_context.instance, &debugInfo, _context.allocator, &_debugMessenger) != VK_SUCCESS) {
            OBELISK_ERROR("failed to set up debug messenger!");
            return OBELISK_FAIL;
        }
    }

    /* create surface */
    if (obeliskCreateWindowSurface(window, _context.instance, &_context.surface) != VK_SUCCESS) {
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

    if (vkCreateCommandPool(_context.device, &info, _context.allocator, &_context.commandPool) != VK_SUCCESS) {
        OBELISK_ERROR("failed to create command pool!");
        return OBELISK_FAIL;
    }

    return OBELISK_OK;
}

void obeliskDestroyContext() {
    /* destroy command pool */
    vkDestroyCommandPool(_context.device, _context.commandPool, _context.allocator);

    /* destroy device */
    vkDestroyDevice(_context.device, _context.allocator);

    /* destroy debug messenger */
    if (_debugMessenger != VK_NULL_HANDLE)
        vkDestroyDebugUtilsMessengerEXT(_context.instance, _debugMessenger, _context.allocator);

    /* destroy surface and instance */
    vkDestroySurfaceKHR(_context.instance, _context.surface, _context.allocator);
    vkDestroyInstance(_context.instance, _context.allocator);
}

ObeliskContext*  obeliskGetContext() { return &_context; }
VkDevice         obeliskGetDevice()             { return _context.device; }
VkPhysicalDevice obeliskGetPhysicalDevice()     { return _context.physicalDevice; }
VkSurfaceKHR     obeliskGetSurface()            { return _context.surface; }
VkQueue          obeliskGetGraphicsQueue()      { return _context.queues[OBELISK_QUEUE_GRAPHICS]; }
VkQueue          obeliskGetPresentQueue()       { return _context.queues[OBELISK_QUEUE_PRESENT]; }
uint32_t*        obeliskGetQueueFamilyIndices() { return _context.queueFamilyIndices; }

VkSharingMode obeliskGetImageSharingMode() {
    if (_context.queueFamilyIndices[OBELISK_QUEUE_GRAPHICS] != _context.queueFamilyIndices[OBELISK_QUEUE_PRESENT])
        return VK_SHARING_MODE_CONCURRENT;

    return VK_SHARING_MODE_EXCLUSIVE;
}

uint32_t obeliskGetQueueGraphicsFamilyIndex(const ObeliskContext* context) {
    return context->queueFamilyIndices[OBELISK_QUEUE_GRAPHICS];
}

uint32_t obeliskGetQueuePresentFamilyIndex(const ObeliskContext* context) {
    return context->queueFamilyIndices[OBELISK_QUEUE_PRESENT];
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