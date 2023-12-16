#include "ignis.h"


#include "minimal/minimal.h"

static IgnisContext context;

uint8_t ignisInit(const IgnisPlatform* platform)
{
    if (!ignisCreateContext(&context, "IgnisApp", platform))
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


uint8_t ignisBeginFrame()
{
    return IGNIS_OK;
}

uint8_t ignisEndFrame()
{
    return IGNIS_OK;
}
