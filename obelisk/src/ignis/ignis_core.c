#include "ignis_core.h"

#include <stdlib.h>

#include "minimal/common.h"

void* ignisAlloc(size_t size) { return malloc(size); }
void  ignisFree(void* block, size_t size)  { free(block); }


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

void ignisFillDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT* info)
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

VkResult ignisCreateDebugMessenger(IgnisContext* context)
{
    VkDebugUtilsMessengerCreateInfoEXT info = { 0 };
    ignisFillDebugMessengerCreateInfo(&info);

    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context->instance, "vkCreateDebugUtilsMessengerEXT");
    if (func) return func(context->instance, &info, context->allocator, &context->debug_messenger);

    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void ignisDestroyDebugMessenger(IgnisContext* context)
{
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context->instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func) func(context->instance, context->debug_messenger, context->allocator);
}

#endif