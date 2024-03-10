#include "ignis.h"

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

    const uint32_t extensionCount = sizeof(extensions) / sizeof(extensions[0]);
    // const char* const extensions = platform->queryExtensions(&extensionCount);

    if (!ignisCreateInstance(name, extensions, extensionCount))
    {
        IGNIS_ERROR("Failed to create instance");
        return IGNIS_FAIL;
    }

    // Surface
    VkSurfaceKHR surface;
    VkResult result = platform->createSurface(ignisGetVkInstance(), platform->context, ignisGetAllocator(), &surface);
    if (result != VK_SUCCESS)
    {
        IGNIS_ERROR("Failed to create window surface with result: %u", result);
        return IGNIS_FAIL;
    }

    VkExtent2D extent = { 1280, 720 };
    return ignisCreateContext(surface, extent);
}

void ignisTerminate()
{
    ignisDestroyContext();
}

/* ---------------------| color |----------------------------------------------*/
IgnisColorRGBA IGNIS_WHITE = { 1.0f, 1.0f, 1.0f, 1.0f };
IgnisColorRGBA IGNIS_BLACK = { 0.0f, 0.0f, 0.0f, 1.0f };
IgnisColorRGBA IGNIS_RED = { 1.0f, 0.0f, 0.0f, 1.0f };
IgnisColorRGBA IGNIS_GREEN = { 0.0f, 1.0f, 0.0f, 1.0f };
IgnisColorRGBA IGNIS_BLUE = { 0.0f, 0.0f, 1.0f, 1.0f };
IgnisColorRGBA IGNIS_CYAN = { 0.0f, 1.0f, 1.0f, 1.0f };
IgnisColorRGBA IGNIS_MAGENTA = { 1.0f, 0.0f, 1.0f, 1.0f };
IgnisColorRGBA IGNIS_YELLOW = { 1.0f, 1.0f, 0.0f, 1.0f };

IgnisColorRGBA IGNIS_DARK_GREY = { 0.2f, 0.2f, 0.2f, 1.0f };
IgnisColorRGBA IGNIS_LIGHT_GREY = { 0.75f, 0.75f, 0.75f, 1.0f };

IgnisColorRGBA* ignisBlendColorRGBA(IgnisColorRGBA* color, float alpha)
{
    color->a = alpha;
    return color;
}