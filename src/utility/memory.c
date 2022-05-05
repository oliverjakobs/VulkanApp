#include "memory.h"

#include <string.h>

#define OBELISK_BLOCK_HDR(block)   ((size_t*)(void*)(block) - 1)
#define OBELISK_BLOCK_SIZE(block)  OBELISK_BLOCK_HDR(block)[0]

typedef struct {
    size_t allocated;
} ObeliskMemoryTrace;

static ObeliskMemoryTrace memTrace = { 0 };

void* obeliskAllocate(size_t size) {
    size_t* hdr = malloc(size + sizeof(size_t));

    if (!hdr) return NULL;

    memTrace.allocated += size;
    
    hdr[0] = size;
    return hdr + 1;
}

void* obeliskReallocate(void* block, size_t size) {
    size_t oldSize = block ? OBELISK_BLOCK_SIZE(block) : 0;
    size_t* hdr = realloc(block ? OBELISK_BLOCK_HDR(block) : NULL, size + sizeof(size_t));

    if (!hdr) return NULL;

    memTrace.allocated += (size - oldSize);
    
    hdr[0] = size;
    return hdr + 1;
}

void obeliskFree(void* block) {
    if (!block) return;

    size_t* hdr = OBELISK_BLOCK_HDR(block);
    size_t size = OBELISK_BLOCK_SIZE(block);

    if (size > memTrace.allocated)
        OBELISK_WARN("Freeing %llu untraced bytes", size - memTrace.allocated);

    free(hdr);

    memTrace.allocated -= size;
}

void* obeliskMemDup(const void* src, size_t size) {
    void* dst = obeliskAllocate(size);
    return dst ? memcpy(dst, src, size) : NULL;
}

size_t obeliskMemoryGetBytes() { return memTrace.allocated; }