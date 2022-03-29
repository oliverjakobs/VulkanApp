#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "common.h"
#include "Buffer.h"

#define MAX_FRAMES_IN_FLIGHT 2

typedef struct {
    VkSwapchainKHR handle;

    VkImage* images;
    VkImageView* views;
    VkFramebuffer* framebuffers;
    uint32_t count;

    VkFormat format;
    VkExtent2D extent;
    VkRenderPass renderPass;

    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;

    /* frame data */
    VkSemaphore imageAvailable[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore renderFinished[MAX_FRAMES_IN_FLIGHT];
    VkFence fences[MAX_FRAMES_IN_FLIGHT];

    VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
    VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
    Buffer uniformBuffers[MAX_FRAMES_IN_FLIGHT];
} ObeliskSwapchain;

int createSwapchain(ObeliskSwapchain* swapchain, VkSwapchainKHR oldSwapchain, uint32_t width, uint32_t height);
int recreateSwapchain(ObeliskSwapchain* swapchain, uint32_t width, uint32_t height);
void destroySwapchain(ObeliskSwapchain* swapchain);

int createRenderPass(ObeliskSwapchain* swapchain);
int createFramebuffers(ObeliskSwapchain* swapchain);

int acquireSwapchainImage(ObeliskSwapchain* swapchain, uint32_t frame, uint32_t* imageIndex);
int submitFrame(ObeliskSwapchain* swapchain, VkCommandBuffer cmdBuffer, uint32_t frame);
int presentFrame(ObeliskSwapchain* swapchain, uint32_t imageIndex, uint32_t frame);

void commandBufferStart(VkCommandBuffer cmdBuffer, const ObeliskSwapchain* swapchain, uint32_t imageIndex);
void commandBufferEnd(VkCommandBuffer cmdBuffer);

int createDescriptorPool(ObeliskSwapchain* swapchain);
void destroyDescriptorPool(ObeliskSwapchain* swapchain);

int createDescriptorLayout(ObeliskSwapchain* swapchain);
void destroyDescriptorLayout(ObeliskSwapchain* swapchain);

int createDescriptorSets(ObeliskSwapchain* swapchain);

int createSyncObjects(ObeliskSwapchain* swapchain);
void destroySyncObjects(ObeliskSwapchain* swapchain);

#endif // !SWAPCHAIN_H