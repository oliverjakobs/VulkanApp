#ifndef OBELISK_BUFFER_H
#define OBELISK_BUFFER_H

#include "core/core.h"

typedef struct {
    VkBuffer handle;
    VkDeviceMemory memory;
    VkDeviceSize size;
} ObeliskBuffer;

int obeliskCreateVertexBuffer(ObeliskBuffer* buffer, const void* vertices, uint32_t count, uint32_t vertexSize);
int obeliskCreateIndexBuffer(ObeliskBuffer* buffer, const uint32_t* indices, uint32_t count);
int obeliskCreateUniformBuffer(ObeliskBuffer* buffer, VkDeviceSize size);
void obeliskDestroyBuffer(ObeliskBuffer* buffer);

void obeliskWriteBuffer(ObeliskBuffer* buffer, const void* src, VkDeviceSize size);

#endif // !OBELISK_BUFFER_H
