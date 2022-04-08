#ifndef OBELISK_MEMORY_H
#define OBELISK_MEMORY_H

#include "../common.h"

void* obeliskAllocate(size_t size);
void* obeliskReallocate(void* block, size_t size);

void obeliskFree(void* block);

void* obeliskMemDup(const void* src, size_t size);

size_t obeliskMemoryGetBytes();

#endif // !OBELISK_MEMORY_H
