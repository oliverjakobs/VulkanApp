#ifndef IGNIS_H
#define IGNIS_H

#include "ignis_core.h"

uint8_t ignisInit(const char* name, const IgnisPlatform* platform);
void ignisTerminate();


#endif /* !IGNIS_H */
