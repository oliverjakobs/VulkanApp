#include "ignis.h"


#include "minimal/minimal.h"


static IgnisContext context;

uint8_t ignisInit(const char* name, const IgnisPlatform* platform)
{
    if (!ignisCreateContext(&context, name, platform))
    {
        MINIMAL_CRITICAL("failed to create context");
        return IGNIS_FAIL;
    }

    return IGNIS_OK;
}

void ignisTerminate()
{
    ignisDestroyContext(&context);
}

void ignisSetClearColor(float r, float g, float b, float a)
{
    context.clearColor.r = r;
    context.clearColor.g = g;
    context.clearColor.b = b;
    context.clearColor.a = a;
}


uint8_t ignisBeginFrame()
{
    return IGNIS_OK;
}

uint8_t ignisEndFrame()
{
    return IGNIS_OK;
}
