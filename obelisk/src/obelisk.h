#ifndef OBELISK_H
#define OBELISK_H

#include "minimal/minimal.h"
#include "ignis/ignis.h"

MINIMAL_API u8 obeliskLoad(MinimalApp* app, const char* title,  i32 x, i32 y, u32 w, u32 h);
MINIMAL_API void obeliskDestroy(MinimalApp* app);

MINIMAL_API void obeliskRun(MinimalApp* app);

#endif /* !OBELISK_H */