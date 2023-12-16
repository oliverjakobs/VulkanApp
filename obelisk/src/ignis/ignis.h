#ifndef IGNIS_H
#define IGNIS_H

#include "ignis_core.h"

uint8_t ignisInit(const IgnisPlatform* platform);
void ignisTerminate();


uint8_t ignisBeginFrame();
uint8_t ignisEndFrame();




#endif /* !IGNIS_H */
