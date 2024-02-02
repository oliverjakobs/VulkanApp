#ifndef OBELISK_H
#define OBELISK_H

#include <minimal/minimal.h>

// Unsigned int types.
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

// Signed int types.
typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

// Floating point types
typedef float f32;
typedef double f64;

// Properly define static assertions.
#if defined(__clang__) || defined(__gcc__)
#define MINIMAL_STATIC_ASSERT _Static_assert
#else
#define MINIMAL_STATIC_ASSERT static_assert
#endif

// Ensure all types are of the correct size.
MINIMAL_STATIC_ASSERT(sizeof(u8) == 1, "Expected u8 to be 1 byte.");
MINIMAL_STATIC_ASSERT(sizeof(u16) == 2, "Expected u16 to be 2 bytes.");
MINIMAL_STATIC_ASSERT(sizeof(u32) == 4, "Expected u32 to be 4 bytes.");
MINIMAL_STATIC_ASSERT(sizeof(u64) == 8, "Expected u64 to be 8 bytes.");

MINIMAL_STATIC_ASSERT(sizeof(i8) == 1, "Expected i8 to be 1 byte.");
MINIMAL_STATIC_ASSERT(sizeof(i16) == 2, "Expected i16 to be 2 bytes.");
MINIMAL_STATIC_ASSERT(sizeof(i32) == 4, "Expected i32 to be 4 bytes.");
MINIMAL_STATIC_ASSERT(sizeof(i64) == 8, "Expected i64 to be 8 bytes.");

MINIMAL_STATIC_ASSERT(sizeof(f32) == 4, "Expected f32 to be 4 bytes.");
MINIMAL_STATIC_ASSERT(sizeof(f64) == 8, "Expected f64 to be 8 bytes.");

typedef struct ObeliskApp ObeliskApp;

typedef u8   (*ObeliskLoadCB)    (ObeliskApp* app, u32 w, u32 h);
typedef void (*ObeliskDestroyCB) (ObeliskApp* app);

typedef u8   (*ObeliskEventCB)   (ObeliskApp* app, const MinimalEvent* e);
typedef void (*ObeliskTickCB)    (ObeliskApp* app, f32 deltatime);

struct ObeliskApp
{
    MinimalWindow* window;
    ObeliskLoadCB    on_load;
    ObeliskDestroyCB on_destroy;

    ObeliskEventCB on_event;
    ObeliskTickCB  on_tick;
};

MINIMAL_API u8 obeliskLoad(ObeliskApp* app, const char* title,  i32 x, i32 y, u32 w, u32 h);
MINIMAL_API void obeliskDestroy(ObeliskApp* app);

MINIMAL_API void obeliskRun(ObeliskApp* app);

#endif /* !OBELISK_H */