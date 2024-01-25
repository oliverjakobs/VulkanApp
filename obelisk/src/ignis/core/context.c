#include "../ignis_core.h"

#include "minimal/common.h"


#ifdef IGNIS_DEBUG

VKAPI_ATTR VkBool32 VKAPI_CALL ignisDebugUtilsMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data)
{
    switch (message_severity)
    {
        default:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            MINIMAL_ERROR(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            MINIMAL_WARN(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            MINIMAL_INFO(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            MINIMAL_TRACE(callback_data->pMessage);
            break;
    }
    return VK_FALSE;
}

#endif

static const char* const VALIDATION_LAYERS[] = {
    "VK_LAYER_KHRONOS_validation"
};

static const uint32_t VALIDATION_LAYER_COUNT = sizeof(VALIDATION_LAYERS) / sizeof(VALIDATION_LAYERS[0]);

static uint8_t ignisCheckValidationLayerSupport()
{
    uint32_t count;
    vkEnumerateInstanceLayerProperties(&count, NULL);
    if (!count || VALIDATION_LAYER_COUNT > count) return IGNIS_FAIL;

    VkLayerProperties* properties = ignisAlloc(sizeof(VkLayerProperties) * count);
    if (!properties) return IGNIS_FAIL;

    vkEnumerateInstanceLayerProperties(&count, properties);

    uint8_t found = IGNIS_FAIL;
    for (size_t i = 0; i < VALIDATION_LAYER_COUNT; ++i)
    {
        found = IGNIS_FAIL;
        for (size_t j = 0; j < count; ++j)
        {
            if (strcmp(VALIDATION_LAYERS[i], properties[j].layerName) == 0)
            {
                found = IGNIS_OK;
                break;
            }
        }

        if (!found) break;
    }

    ignisFree(properties, sizeof(VkLayerProperties) * count);
    return found;
}

uint8_t ignisCreateContext(IgnisContext* context, const char* name, const IgnisPlatform* platform)
{
    const VkAllocationCallbacks* allocator = ignisGetAllocator();

#ifdef IGNIS_DEBUG
    if (!ignisCheckValidationLayerSupport())
    {
        MINIMAL_ERROR("validation layers requested, but not available!");
        return IGNIS_FAIL;
    }
#endif

    // TODO: make platform agnostic
    const char* const extensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        "VK_KHR_win32_surface",
#ifdef IGNIS_DEBUG
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
    };

    const uint32_t extensionCount = sizeof(extensions) / sizeof(extensions[0]);
    // const char* const extensions = platform->queryExtensions(&extensionCount);

    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .apiVersion = VK_API_VERSION_1_2,
        .pApplicationName = name,
        .applicationVersion = VK_MAKE_API_VERSION(1,0,0,0),
        .pEngineName = "ignis",
        .engineVersion = VK_MAKE_API_VERSION(1,0,0,0)
    };

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = extensionCount,
        .ppEnabledExtensionNames = extensions,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = 0
    };

#ifdef IGNIS_DEBUG
    createInfo.enabledLayerCount = VALIDATION_LAYER_COUNT;
    createInfo.ppEnabledLayerNames = VALIDATION_LAYERS;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
                        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
                        // | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                    | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
                    | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
        .pfnUserCallback = ignisDebugUtilsMessengerCallback
    
    };
    createInfo.pNext = &debugCreateInfo;
#endif

    VkResult result = vkCreateInstance(&createInfo, ignisGetAllocator(), &context->instance);
    if (result != VK_SUCCESS)
    {
        MINIMAL_ERROR("vkCreateInstance failed with result: %u", result);
        return IGNIS_FAIL;
    }

    // Debugger
#ifdef IGNIS_DEBUG
    PFN_vkCreateDebugUtilsMessengerEXT func = IGNIS_VK_PFN(context->instance, vkCreateDebugUtilsMessengerEXT);
    if (func)
    {
        result = func(context->instance, &debugCreateInfo, allocator, &context->debugMessenger);
        if (result != VK_SUCCESS)
            MINIMAL_WARN("Failed to create debug messenger with result: %u", result);
    }
    else
    {
        MINIMAL_WARN("Could not find function 'vkCreateDebugUtilsMessengerEXT'");
    }
#endif

    // Surface
    result = platform->createSurface(context->instance, platform->context, ignisGetAllocator(), &context->surface);
    if (result != VK_SUCCESS)
    {
        MINIMAL_ERROR("Failed to create window surface with result: %u", result);
        return IGNIS_FAIL;
    }
    if (!ignisCreateDevice(context->instance, context->surface, &context->device))
    {
        MINIMAL_CRITICAL("failed to create device");
        return IGNIS_FAIL;
    }

    VkExtent2D extent = { 1280, 720 };
    if (!ignisCreateSwapchain(&context->device, context->surface, VK_NULL_HANDLE, extent, &context->swapchain))
    {
        MINIMAL_CRITICAL("failed to create swapchain");
        return IGNIS_FAIL;
    }

    /* allocate command buffers */
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = context->device.commandPool,
        .commandBufferCount = IGNIS_MAX_FRAMES_IN_FLIGHT
    };

    if (vkAllocateCommandBuffers(context->device.handle, &allocInfo, context->commandBuffers) != VK_SUCCESS)
    {
        MINIMAL_ERROR("Failed to allocate command buffers");
        return IGNIS_FAIL;
    }

    if (!ignisCreateSwapchainSyncObjects(context->device.handle, &context->swapchain))
    {
        MINIMAL_CRITICAL("failed to create swapchain sync objects");
        return IGNIS_FAIL;
    }

    context->currentFrame = 0;
    context->imageIndex = 0;

    context->swapchainGeneration = 0;
    context->swapchainLastGeneration = 0;
    return IGNIS_OK;
}

void ignisDestroyContext(IgnisContext* context)
{
    const VkAllocationCallbacks* allocator = ignisGetAllocator();

    ignisDestroySwapchainSyncObjects(context->device.handle, &context->swapchain);
    vkFreeCommandBuffers(context->device.handle, context->device.commandPool, IGNIS_MAX_FRAMES_IN_FLIGHT, context->commandBuffers);

    ignisDestroySwapchain(context->device.handle, &context->swapchain);
    ignisDestroyDevice(&context->device);

    vkDestroySurfaceKHR(context->instance, context->surface, allocator);

#ifdef IGNIS_DEBUG

    PFN_vkDestroyDebugUtilsMessengerEXT func = IGNIS_VK_PFN(context->instance, vkDestroyDebugUtilsMessengerEXT);
    if (func)
        func(context->instance, context->debugMessenger, allocator);
    else
        MINIMAL_WARN("Could not find function 'vkDestroyDebugUtilsMessengerEXT'");

#endif

    vkDestroyInstance(context->instance, allocator);
}


const VkAllocationCallbacks* ignisGetAllocator() { return NULL; }