#include "ignis.h"


uint8_t ignisInit(const char* name, const IgnisPlatform* platform)
{
    return ignisCreateContext(name, platform);
}

void ignisTerminate()
{
    ignisDestroyContext();
}
