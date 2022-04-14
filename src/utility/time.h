#ifndef OBELISK_TIME_H
#define OBELISK_TIME_H

#include "../common.h"

typedef struct {
    uint32_t frames;
    uint32_t fps;

    double seconds;
    double deltatime;
    double lastframe;
} ObeliskTimer;

void obeliskResetTimer(ObeliskTimer* timer);

void obeliskStartTimer(ObeliskTimer* timer, double seconds);
void obeliskEndTimer(ObeliskTimer* timer, double seconds);

#endif // !OBELISK_TIME_H
