#include "ignis.h"
#include "types.h"

#include <string.h>

#include "obelisk_memory.h"
#include "minimal/minimal.h"

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data);

static const char* const validation_layers[] = {
    "VK_LAYER_KHRONOS_validation"
};

static const uint32_t validation_layer_count = sizeof(validation_layers) / sizeof(validation_layers[0]);

static int ignisCheckValidationLayerSupport()
{
    uint32_t count;
    vkEnumerateInstanceLayerProperties(&count, NULL);
    if (!count || validation_layer_count > count) return 0;

    VkLayerProperties* layers = obeliskAlloc(sizeof(VkLayerProperties) * count, OBELISK_MEMTAG_UNTRACED);
    if (!layers) return 0;

    vkEnumerateInstanceLayerProperties(&count, layers);

    int found = 0;
    for (size_t i = 0; i < validation_layer_count; ++i)
    {
        found = 0;
        for (size_t available = 0; available < count; ++available)
        {
            if (strcmp(validation_layers[i], layers[available].layerName) == 0)
            {
                found = 1;
                break;
            }
        }

        if (!found) break;
    }

    obeliskFree(layers, sizeof(VkLayerProperties) * count, OBELISK_MEMTAG_UNTRACED);
    return found;
}

static IgnisContext context;

uint8_t ignisCreateContext(const char* name)
{

#ifdef IGNIS_DEBUG
    if (!ignisCheckValidationLayerSupport())
    {
        MINIMAL_ERROR("validation layers requested, but not available!");
        return IGNIS_FAIL;
    }
#endif

    VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .apiVersion = VK_API_VERSION_1_2,
        .pApplicationName = name,
        .applicationVersion = VK_MAKE_API_VERSION(1,0,0,0),
        .pEngineName = "ignis",
        .engineVersion = VK_MAKE_API_VERSION(1,0,0,0)
    };

    // TODO: make platform agnostic
    const char* const extensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        "VK_KHR_win32_surface",
#ifdef IGNIS_DEBUG
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
    };
    
    const uint32_t extension_count = sizeof(extensions) / sizeof(extensions[0]);

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
    
    /* print all required extensions */
    MINIMAL_INFO("Required extensions:");
    for (size_t i = 0; i < extension_count; ++i)
    {
        MINIMAL_INFO(" - %s", extensions[i]);
    }
#endif

    VkResult result = vkCreateInstance(&create_info, context.allocator, &context.instance);
    if (result != VK_SUCCESS)
    {
        MINIMAL_ERROR("vkCreateInstance failed with result: %u", result);
        return IGNIS_FAIL;
    }

        // Debugger
#ifdef IGNIS_DEBUG
    u32 severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
                    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
                    // | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = severity,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                    | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
                    | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
        .pfnUserCallback = vk_debug_callback
    };

    PFN_vkCreateDebugUtilsMessengerEXT fn = VK_EXT_PFN(context.instance, vkCreateDebugUtilsMessengerEXT);
    MINIMAL_ASSERT(fn, "Extension missing!");
    VK_CHECK(fn(context.instance, &debug_create_info, context.allocator, &context.debug_messenger));
#endif

    return IGNIS_OK;
}

void ignisDestroyContext()
{
#ifdef IGNIS_DEBUG
    PFN_vkDestroyDebugUtilsMessengerEXT fn = VK_EXT_PFN(context.instance, vkDestroyDebugUtilsMessengerEXT);
    MINIMAL_ASSERT(fn, "Extension missing!");
    fn(context.instance, context.debug_messenger, context.allocator);
#endif

    vkDestroyInstance(context.instance, context.allocator);
}

uint8_t ignisBeginFrame()
{
    return IGNIS_OK;
}

uint8_t ignisEndFrame()
{
    return IGNIS_OK;
}

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
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