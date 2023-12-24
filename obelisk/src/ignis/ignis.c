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
    context.renderPass.clearColor.r = r;
    context.renderPass.clearColor.g = g;
    context.renderPass.clearColor.b = b;
    context.renderPass.clearColor.a = a;
}


uint8_t ignisBeginFrame()
{
    return IGNIS_OK;
}

uint8_t ignisEndFrame()
{
    return IGNIS_OK;
}
