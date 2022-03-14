#include "common.h"

#include <stdio.h>
#include "Minimal/Utils.h"

uint32_t clamp32(uint32_t val, uint32_t min, uint32_t max) {
    const uint32_t t = val < min ? min : val;
    return t > max ? max : t;
}

char* readSPIRV(const char* path, size_t* sizeptr) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        MINIMAL_ERROR("Failed to open file: %s", path);
        return NULL;
    }

    /* find file size */
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    char* buffer = malloc(size);
    if (!buffer) {
        MINIMAL_ERROR("Failed to allocate memory for file: %s", path);
        fclose(file);
        return NULL;
    }

    if (fread(buffer, size, 1, file) != 1) {
        MINIMAL_ERROR("Failed to read file: %s", path);
        free(buffer);
        fclose(file);
        return NULL;
    }

    if (sizeptr) *sizeptr = size;

    fclose(file);
    return buffer;
}