#ifndef FRAME_H
#define FRAME_H

#include "common.h"

int createCommandPool(VulkanContext* context);
int createCommandBuffer(VulkanContext* context);

int createSyncObjects(VulkanContext* context);
void destroySyncObjects(VulkanContext* context);

void commandBufferStart(VkCommandBuffer cmdBuffer, const Swapchain* swapchain, uint32_t imageIndex);
void commandBufferEnd(VkCommandBuffer cmdBuffer);

#endif // !FRAME_H
