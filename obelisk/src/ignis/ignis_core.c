#include "ignis_core.h"

#include "minimal/common.h"


#ifdef IGNIS_DEBUG

static void ignisPopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT* info);

static VkResult ignisCreateDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT* messenger);
static void ignisDestroyDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT messenger);

#endif

static const char* const validation_layers[] = {
    "VK_LAYER_KHRONOS_validation"
};

static const uint32_t validation_layer_count = sizeof(validation_layers) / sizeof(validation_layers[0]);

static uint8_t ignisCheckValidationLayerSupport()
{
    uint32_t count;
    vkEnumerateInstanceLayerProperties(&count, NULL);
    if (!count || validation_layer_count > count) return IGNIS_FAIL;

    VkLayerProperties* properties = ignisAlloc(sizeof(VkLayerProperties) * count);
    if (!properties) return IGNIS_FAIL;

    vkEnumerateInstanceLayerProperties(&count, properties);

    uint8_t found = IGNIS_FAIL;
    for (size_t i = 0; i < validation_layer_count; ++i)
    {
        found = IGNIS_FAIL;
        for (size_t j = 0; j < count; ++j)
        {
            if (strcmp(validation_layers[i], properties[j].layerName) == 0)
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

    const uint32_t extension_count = sizeof(extensions) / sizeof(extensions[0]);
    // const char* const extensions = platform->queryExtensions(&extension_count);

    VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .apiVersion = VK_API_VERSION_1_2,
        .pApplicationName = name,
        .applicationVersion = VK_MAKE_API_VERSION(1,0,0,0),
        .pEngineName = "ignis",
        .engineVersion = VK_MAKE_API_VERSION(1,0,0,0)
    };

    VkInstanceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .enabledExtensionCount = extension_count,
        .ppEnabledExtensionNames = extensions,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = 0
    };

#ifdef IGNIS_DEBUG
    create_info.enabledLayerCount = validation_layer_count;
    create_info.ppEnabledLayerNames = validation_layers;

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = { 0 };
    ignisPopulateDebugMessengerCreateInfo(&debug_create_info);
    create_info.pNext = &debug_create_info;
#endif

    VkResult result = vkCreateInstance(&create_info, ignisGetAllocator(), &context->instance);
    if (result != VK_SUCCESS)
    {
        MINIMAL_ERROR("vkCreateInstance failed with result: %u", result);
        return IGNIS_FAIL;
    }

    // Debugger
#ifdef IGNIS_DEBUG
    result = ignisCreateDebugMessenger(context->instance, &context->debug_messenger);
    if (result != VK_SUCCESS)
    {
        MINIMAL_ERROR("Failed to create debug messenger with result: %u", result);
        return IGNIS_FAIL;
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
    
    ignisPrintDeviceInfo(&context->device);

    if (!ignisCreateSwapchain(&context->device, context->surface, VK_NULL_HANDLE, 1280, 720, &context->swapchain))
    {
        MINIMAL_CRITICAL("failed to create swapchain");
        return IGNIS_FAIL;
    }

    return IGNIS_OK;
}

void ignisDestroyContext(IgnisContext* context)
{
    ignisDestroySwapchain(&context->device, &context->swapchain);
    ignisDestroyDevice(&context->device);

    vkDestroySurfaceKHR(context->instance, context->surface, ignisGetAllocator());

#ifdef IGNIS_DEBUG
    ignisDestroyDebugMessenger(context->instance, context->debug_messenger);
#endif

    vkDestroyInstance(context->instance, ignisGetAllocator());
}


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

void ignisPopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT* info)
{
    info->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    info->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
                        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
                        // | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    info->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                    | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
                    | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    info->pfnUserCallback = ignisDebugUtilsMessengerCallback;
}

VkResult ignisCreateDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT* messenger)
{
    VkDebugUtilsMessengerCreateInfoEXT create_info = { 0 };
    ignisPopulateDebugMessengerCreateInfo(&create_info);

    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func) return func(instance, &create_info, ignisGetAllocator(), messenger);

    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void ignisDestroyDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT messenger)
{
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func) func(instance, messenger, ignisGetAllocator());
}

#endif

const VkAllocationCallbacks* ignisGetAllocator() { return NULL; }