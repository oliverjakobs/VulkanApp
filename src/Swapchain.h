#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "common.h"
#include "Buffer.h"

#define MAX_FRAMES_IN_FLIGHT 2

typedef struct {
    VkSwapchainKHR handle;

    VkFormat imageFormat;
    VkExtent2D extent;

    uint32_t imageCount;
    VkImage* images;
    VkImageView* imageViews;

    VkFramebuffer* framebuffers;
    VkRenderPass renderPass;

    /* frame data */
    VkSemaphore imageAvailable[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore renderFinished[MAX_FRAMES_IN_FLIGHT];
    VkFence fences[MAX_FRAMES_IN_FLIGHT];

    /* TODO move to renderer */
    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;

    VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
    VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
    Buffer uniformBuffers[MAX_FRAMES_IN_FLIGHT];
} ObeliskSwapchain;

int obeliskCreateSwapchain(ObeliskSwapchain* swapchain, VkSwapchainKHR oldSwapchain, uint32_t width, uint32_t height);
int obeliskRecreateSwapchain(ObeliskSwapchain* swapchain, uint32_t width, uint32_t height);
void obeliskDestroySwapchain(ObeliskSwapchain* swapchain);

int obeliskCreateSyncObjects(ObeliskSwapchain* swapchain);
void obeliskDestroySyncObjects(ObeliskSwapchain* swapchain);

int obeliskAcquireSwapchainImage(ObeliskSwapchain* swapchain, uint32_t frame, uint32_t* imageIndex);

int obeliskSubmitFrame(ObeliskSwapchain* swapchain, VkCommandBuffer cmdBuffer, uint32_t frame);
int obeliskPresentFrame(ObeliskSwapchain* swapchain, uint32_t imageIndex, uint32_t frame);

/* TODO move to renderer */
void commandBufferStart(VkCommandBuffer cmdBuffer, const ObeliskSwapchain* swapchain, uint32_t imageIndex);
void commandBufferEnd(VkCommandBuffer cmdBuffer);

int createDescriptorPool(ObeliskSwapchain* swapchain);
void destroyDescriptorPool(ObeliskSwapchain* swapchain);

int createDescriptorLayout(ObeliskSwapchain* swapchain);
void destroyDescriptorLayout(ObeliskSwapchain* swapchain);

int createDescriptorSets(ObeliskSwapchain* swapchain);

#endif // !SWAPCHAIN_H