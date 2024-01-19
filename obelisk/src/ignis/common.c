#include "common.h"

#include <stdlib.h>
#include <stdio.h>

#include "minimal/common.h"

void* ignisAlloc(size_t size) { return malloc(size); }
void  ignisFree(void* block, size_t size)  { free(block); }

char* ignisReadFile(const char* path, size_t* sizeptr)
{
    FILE* file = fopen(path, "rb");
    if (!file)
    {
        MINIMAL_ERROR("[Ignis] Failed to open file: %s", path);
        return NULL;
    }

    /* find file size */
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);


    char* buffer = ignisAlloc(size + 1);
    if (buffer)
    {
        if (fread(buffer, size, 1, file) != 1)
        {
            //IGNIS_ERROR("[Ignis] Failed to read file: %s", path);
            ignisFree(buffer, size + 1);
            fclose(file);
            return NULL;
        }

        buffer[size] = '\0'; /* zero terminate buffer */
        if (sizeptr) *sizeptr = size + 1;
    }
    else
    {
        //IGNIS_ERROR("[Ignis] Failed to allocate memory for file: %s", path);
    }

    fclose(file);
    return buffer;
}

uint32_t ignisClamp32(uint32_t val, uint32_t min, uint32_t max)
{
    const uint32_t t = val < min ? min : val;
    return t > max ? max : t;
}