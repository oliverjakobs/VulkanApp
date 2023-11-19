#include "obelisk_memory.h"

#include <stdlib.h>
#include <string.h>

static struct MemStats
{
    u64 total_allocated;
    u64 tagged_allocations[OBELISK_MEMTAG_COUNT];
} stats;


b8 obeliskMemoryInit()
{
    obeliskMemZero(&stats, sizeof(stats));
    return TRUE;
}

b8 obeliskMemoryTerminate()
{
    return TRUE;
}

KAPI void* obeliskAlloc(u64 size, ObeliskMemTag tag)
{
    if (tag == OBELISK_MEMTAG_UNKNOWN)
        KWARN("obeliskAlloc called with unkown tag.");

    stats.total_allocated += size;
    stats.tagged_allocations[tag] += size;

    void* block = malloc(size);
    return obeliskMemZero(block, size);
}

KAPI void  obeliskFree(void* block, u64 size, ObeliskMemTag tag)
{
    if (tag == OBELISK_MEMTAG_UNKNOWN)
        KWARN("obeliskFree called with unkown tag.");

    stats.total_allocated -= size;
    stats.tagged_allocations[tag] -= size;

    free(block);
}

KAPI void* obeliskMemCopy(void* dst, const void* src, u64 size)
{
    return memcpy(dst, src, size);
}

KAPI void* obeliskMemZero(void* block, u64 size)
{
    return memset(block, 0, size);
}

KAPI void* obeliskMemSet(void* block, i32 value, u64 size)
{
    return memset(block, value, size);
}
