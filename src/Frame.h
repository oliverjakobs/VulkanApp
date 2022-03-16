#ifndef FRAME_H
#define FRAME_H

#include "common.h"
#include "Device.h"

int createCommandPool(VulkanContext* context);
int createCommandBuffer(VulkanContext* context);

int createSyncObjects(VulkanContext* context);
void destroySyncObjects(VulkanContext* context);

void recordCommandBuffer(const VulkanContext* context, VkCommandBuffer cmdBuffer, uint32_t imageIndex);

#endif // !FRAME_H
