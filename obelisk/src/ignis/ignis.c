#include "ignis.h"


#include "minimal/minimal.h"

static IgnisContext context;

uint8_t ignisInit(const char* name, const IgnisPlatform* platform)
{
    // TODO: make platform agnostic
    const char* const extensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        "VK_KHR_win32_surface",
#ifdef IGNIS_DEBUG
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
    };

    const uint32_t extension_count = sizeof(extensions) / sizeof(extensions[0]);

#ifdef IGNIS_DEBUG
    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = { 0 };
    ignisPopulateDebugMessengerCreateInfo(&debug_create_info);

    if (!ignisCreateContext(&context, name, extensions, extension_count, &debug_create_info))
#else
    if (!ignisCreateContext(&context, name, extensions, extension_count, NULL))
#endif
    {
        MINIMAL_CRITICAL("failed to create context");
        return IGNIS_FAIL;
    }

    // Debugger
    VkResult result;
#ifdef IGNIS_DEBUG
    result = ignisCreateDebugMessenger(context.instance, context.allocator, &context.debug_messenger);
    if (result != VK_SUCCESS)
    {
        MINIMAL_ERROR("Failed to create debug messenger with result: %u", result);
        return IGNIS_FAIL;
    }
#endif

    // Surface
    result = platform->create_surface(context.instance, platform->context, context.allocator, &context.surface);
    if (result != VK_SUCCESS)
    {
        MINIMAL_ERROR("Failed to create window surface with result: %u", result);
        return IGNIS_FAIL;
    }

    if (!ignisPickPhysicalDevice(&context))
    {
        MINIMAL_CRITICAL("failed to pick physical device");
        return IGNIS_FAIL;
    }
    
    ignisPrintPhysicalDeviceInfo(context.physical_device);

    if (!ignisCreateLogicalDevice(&context))
    {
        MINIMAL_CRITICAL("failed to create logical device");
        return IGNIS_FAIL;
    }

    if (!ignisCreateSwapchain(&context, &context.swapchain, VK_NULL_HANDLE, 1280, 720))
    {
        MINIMAL_CRITICAL("failed to create swapchain");
        return IGNIS_FAIL;
    }

    return IGNIS_OK;
}

void ignisTerminate()
{
    ignisDestroySwapchain(&context, &context.swapchain);

    vkDestroyDevice(context.device, context.allocator);

    vkDestroySurfaceKHR(context.instance, context.surface, context.allocator);

#ifdef IGNIS_DEBUG
    ignisDestroyDebugMessenger(context.instance, context.debug_messenger, context.allocator);
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
