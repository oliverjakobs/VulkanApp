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
