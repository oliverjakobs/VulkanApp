#include "time.h"

void obeliskResetTimer(ObeliskTimer* timer) {
    timer->seconds = 0.0;
    timer->frames = 0;
    timer->fps = 0;

    timer->deltatime = 0.0;
    timer->lastframe = 0.0;
}

void obeliskStartTimer(ObeliskTimer* timer, double seconds) {
    timer->deltatime = seconds - timer->lastframe;
    timer->lastframe = seconds;
}

void obeliskEndTimer(ObeliskTimer* timer, double seconds) {
    timer->frames++;
    if ((seconds - timer->seconds) > 1.0) {
        timer->seconds += 1.0;
        timer->fps = timer->frames;
        timer->frames = 0;
    }
}