#ifndef IGNIS_H
#define IGNIS_H

#include "ignis_core.h"

uint8_t ignisInit(const char* name, uint32_t width, uint32_t height, const void* platformHandle);
void ignisTerminate();

/*
 * --------------------------------------------------------------
 *                          color
 * --------------------------------------------------------------
 */
typedef struct
{
    float r, g, b, a;
} IgnisColorRGBA;

extern IgnisColorRGBA IGNIS_WHITE;
extern IgnisColorRGBA IGNIS_BLACK;
extern IgnisColorRGBA IGNIS_RED;
extern IgnisColorRGBA IGNIS_GREEN;
extern IgnisColorRGBA IGNIS_BLUE;
extern IgnisColorRGBA IGNIS_CYAN;
extern IgnisColorRGBA IGNIS_MAGENTA;
extern IgnisColorRGBA IGNIS_YELLOW;

extern IgnisColorRGBA IGNIS_DARK_GREY;
extern IgnisColorRGBA IGNIS_LIGHT_GREY;

IgnisColorRGBA* ignisBlendColorRGBA(IgnisColorRGBA* color, float alpha);

#endif /* !IGNIS_H */
