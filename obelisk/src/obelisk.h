#ifndef OBELISK_H
#define OBELISK_H

#include <minimal/minimal.h>

#ifdef OBELISK_EXPORT
// Exports
#ifdef _MSC_VER
#define OBELISK_API __declspec(dllexport)
#else
#define OBELISK_API __attribute__((visibility("default")))
#endif
#else
// Imports
#ifdef _MSC_VER
#define OBELISK_API __declspec(dllimport)
#else
#define OBELISK_API
#endif
#endif

OBELISK_API u8 obeliskLoad(MinimalApp* app, const char* title,  i32 x, i32 y, u32 w, u32 h);
OBELISK_API void obeliskDestroy(MinimalApp* app);

OBELISK_API void obeliskRun(MinimalApp* app);

#endif /* !OBELISK_H */