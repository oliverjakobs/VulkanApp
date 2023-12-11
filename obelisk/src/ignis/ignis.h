#ifndef IGNIS_H
#define IGNIS_H

#include <stdint.h>

#define IGNIS_FAIL    0
#define IGNIS_OK      1

#ifdef _DEBUG
    #define IGNIS_DEBUG
#endif

uint8_t ignisCreateContext(const char* name);
void ignisDestroyContext();

uint8_t ignisBeginFrame();
uint8_t ignisEndFrame();


#endif /* !IGNIS_H */
