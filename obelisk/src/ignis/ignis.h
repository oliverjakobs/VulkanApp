#ifndef IGNIS_H
#define IGNIS_H

#include "minimal/common.h"

typedef struct IgnisContext IgnisContext;

u8 ignisCreateContext(IgnisContext* context, const char* name);
void ignisDestroyContext(IgnisContext* context);

u8 ignisBeginFrame(IgnisContext* context);
u8 ignisEndFrame(IgnisContext* context);

#endif /* !IGNIS_H */
