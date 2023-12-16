#include "ignis_core.h"

#include "minimal/common.h"

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
#endif

    VkResult result = vkCreateInstance(&create_info, context->allocator, &context->instance);
    if (result != VK_SUCCESS)
    {
        MINIMAL_ERROR("vkCreateInstance failed with result: %u", result);
        return IGNIS_FAIL;
    }

    // Debugger
#ifdef IGNIS_DEBUG
    result = ignisCreateDebugMessenger(context);
    if (result != VK_SUCCESS)
    {
        MINIMAL_ERROR("Failed to create debug messenger with result: %u", result);
        return IGNIS_FAIL;
    }
#endif

    // Surface
    result = platform->create_surface(context->instance, platform->context, context->allocator, &context->surface);
    if (result != VK_SUCCESS)
    {
        MINIMAL_ERROR("Failed to create window surface with result: %u", result);
        return IGNIS_FAIL;
    }

    if (!ignisPickPhysicalDevice(context))
    {
        MINIMAL_CRITICAL("failed to pick physical device");
        return IGNIS_FAIL;
    }

    if (!ignisCreateLogicalDevice(context))
    {
        MINIMAL_CRITICAL("failed to create logical device");
        return IGNIS_FAIL;
    }

    return IGNIS_OK;
}

void ignisDestroyContext(IgnisContext* context)
{
    vkDestroyDevice(context->device, context->allocator);

    vkDestroySurfaceKHR(context->instance, context->surface, context->allocator);

#ifdef IGNIS_DEBUG
    ignisDestroyDebugMessenger(context);
#endif

    vkDestroyInstance(context->instance, context->allocator);
}