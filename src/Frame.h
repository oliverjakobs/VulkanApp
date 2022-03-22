#ifndef FRAME_H
#define FRAME_H

#include "common.h"
#include "Core.h"

int createSyncObjects(VulkanContext* context);
void destroySyncObjects(VulkanContext* context);

#endif // !FRAME_H
