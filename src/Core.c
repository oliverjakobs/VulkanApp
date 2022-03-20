#include "Core.h"

#include <string.h>

const char* const validationLayers[] = {
    "VK_LAYER_KHRONOS_validation"
};

const uint32_t validationLayerCount = sizeof(validationLayers) / sizeof(validationLayers[0]);

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data
) {
    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        MINIMAL_ERROR("validation layer: %s", callback_data->pMessage);
    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* info,
    const VkAllocationCallbacks* allocator,
    VkDebugUtilsMessengerEXT* messenger
) {
    PFN_vkVoidFunction func = vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func) return ((PFN_vkCreateDebugUtilsMessengerEXT)func)(instance, info, allocator, messenger);
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT messenger,
    const VkAllocationCallbacks* allocator
) {
    PFN_vkVoidFunction func = vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func) ((PFN_vkDestroyDebugUtilsMessengerEXT)func)(instance, messenger, allocator);
}

char** getRequiredExtensions(int debug, uint32_t* count) {
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

int checkValidationLayerSupport() {
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

int createInstance(VulkanContext* context, GLFWwindow* window, const char* appName, const char* engine, int debug) {
    if (debug && !checkValidationLayerSupport()) {
        MINIMAL_ERROR("validation layers requested, but not available!");
        return MINIMAL_FAIL;
    }

    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = appName,
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = engine,
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0
    };

    uint32_t extension_count = 0;
    char** extensions = getRequiredExtensions(debug, &extension_count);

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .ppEnabledExtensionNames = extensions,
        .enabledExtensionCount = extension_count
    };

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

    if (debug) {
        createInfo.enabledLayerCount = validationLayerCount;
        createInfo.ppEnabledLayerNames = validationLayers;
        createInfo.pNext = &debugInfo;
    }
    else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = NULL;
    }

    VkResult result = vkCreateInstance(&createInfo, NULL, &context->instance);

    free(extensions);

    if (result != VK_SUCCESS) {
        MINIMAL_ERROR("Failed to create vulkan instance!");
        return MINIMAL_FAIL;
    }

    /* setup debug messenger */
    if (debug) {
        if (CreateDebugUtilsMessengerEXT(context->instance, &debugInfo, NULL, &context->debugMessenger) != VK_SUCCESS) {
            MINIMAL_ERROR("failed to set up debug messenger!");
            return MINIMAL_FAIL;
        }
    } else {
        context->debugMessenger = VK_NULL_HANDLE;
    }

    /* create surface */
    if (glfwCreateWindowSurface(context->instance, window, NULL, &context->surface) != VK_SUCCESS) {
        MINIMAL_ERROR("failed to create window surface!");
        return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
}

void destroyInstance(VulkanContext* context) {
    /* destroy debug messenger */
    if (context->debugMessenger != VK_NULL_HANDLE)
        DestroyDebugUtilsMessengerEXT(context->instance, context->debugMessenger, NULL);

    /* destroy surface and instance */
    vkDestroySurfaceKHR(context->instance, context->surface, NULL);
    vkDestroyInstance(context->instance, NULL);
}
