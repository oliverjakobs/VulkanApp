#include "ignis.h"

#include <vulkan/vulkan.h>

struct IgnisContext
{
    VkInstance instance;

    VkAllocationCallbacks* allocator;
};

u8 ignisCreateContext(IgnisContext* context, const char* name)
{
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
        .enabledExtensionCount = 0,
        .ppEnabledExtensionNames = 0,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = 0
    };

    VkResult result = vkCreateInstance(&create_info, context->allocator, &context->instance);
    if (result != VK_SUCCESS)
    {
        MINIMAL_ERROR("vkCreateInstance failed with result: %u", result);
        return MINIMAL_FAIL;
    }

    MINIMAL_INFO("Ignis context created successfully.");
    return MINIMAL_OK;
}

void ignisDestroyContext(IgnisContext* context)
{

}

u8 ignisBeginFrame(IgnisContext* context)
{
    return MINIMAL_OK;
}

u8 ignisEndFrame(IgnisContext* context)
{
    return MINIMAL_OK;
}