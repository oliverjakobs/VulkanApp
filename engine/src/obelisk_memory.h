#pragma once

#include "common.h"

typedef enum
{
    OBELISK_MEMTAG_UNKNOWN = -1,
    OBELISK_MEMTAG_UNTRACED = 0,
    OBELISK_MEMTAG_ARRAY,
    OBELISK_MEMTAG_DARRAY,
    OBELISK_MEMTAG_DICT,
    OBELISK_MEMTAG_RING_QUEUE,
    OBELISK_MEMTAG_BST,
    OBELISK_MEMTAG_STRING,
    OBELISK_MEMTAG_APPLICATION,
    OBELISK_MEMTAG_JOB,
    OBELISK_MEMTAG_TEXTURE,
    OBELISK_MEMTAG_MATERIAL_INSTANCE,
    OBELISK_MEMTAG_RENDERER,
    OBELISK_MEMTAG_GAME,
    OBELISK_MEMTAG_TRANSFORM,
    OBELISK_MEMTAG_ENTITY,
    OBELISK_MEMTAG_ENTITY_NODE,
    OBELISK_MEMTAG_SCENE,

    OBELISK_MEMTAG_COUNT
} ObeliskMemTag;

b8 obeliskMemoryInit();
b8 obeliskMemoryTerminate();

KAPI void* obeliskAlloc(u64 size, ObeliskMemTag tag);
KAPI void  obeliskFree(void* block, u64 size, ObeliskMemTag tag);

KAPI void* obeliskMemCopy(void* dst, const void* src, u64 size);
KAPI void* obeliskMemZero(void* block, u64 size);
KAPI void* obeliskMemSet(void* block, i32 value, u64 size);
