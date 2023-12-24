#ifndef IGNIS_H
#define IGNIS_H

#include "ignis_core.h"

uint8_t ignisInit(const char* name, const IgnisPlatform* platform);
void ignisTerminate();

void ignisSetClearColor(float r, float g, float b, float a);

uint8_t ignisBeginFrame();
uint8_t ignisEndFrame();

#endif /* !IGNIS_H */
