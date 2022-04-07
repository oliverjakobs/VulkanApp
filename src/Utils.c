#include "Utils.h"

char* obeliskReadFile(const char* path, size_t* sizeptr) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        OBELISK_ERROR("Failed to open file: %s", path);
        return NULL;
    }

    /* find file size */
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    char* buffer = malloc(size + 1);
    if (!buffer) {
        OBELISK_ERROR("Failed to allocate memory for file: %s", path);
        fclose(file);
        return NULL;
    }

    if (fread(buffer, size, 1, file) != 1) {
        OBELISK_ERROR("Failed to read file: %s", path);
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[size] = '\0'; /* zero terminate buffer */
    if (sizeptr) *sizeptr = size;

    fclose(file);
    return buffer;
}

/* --------------------------| timer |----------------------------------- */
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