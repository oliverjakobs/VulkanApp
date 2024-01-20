#ifndef IGNIS_H
#define IGNIS_H

#include "ignis_core.h"

uint8_t ignisInit(const char* name, const IgnisPlatform* platform);
void ignisTerminate();

uint8_t ignisResize(uint32_t width, uint32_t height);

void ignisSetClearColor(float r, float g, float b, float a);
void ignisSetDepthStencil(float depth, uint32_t stencil);
void ignisSetViewport(float x, float y, float width, float height);
void ignisSetDepthRange(float nearVal, float farVal);

uint8_t ignisBeginFrame();
uint8_t ignisEndFrame();

IgnisContext* ignisGetContext();

#endif /* !IGNIS_H */
