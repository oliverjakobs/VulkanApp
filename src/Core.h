#ifndef CORE_H
#define CORE_H

#include "common.h"

#define MAX_FRAMES_IN_FLIGHT 2

typedef struct {
    uint32_t familiesSet;
    uint32_t graphicsFamily;
    uint32_t presentFamily;
} QueueFamilyIndices;

typedef struct {
    VkSwapchainKHR handle;

    VkImage* images;
    VkImageView* views;
    VkFramebuffer* framebuffers;
    uint32_t count;

    VkFormat format;
    VkExtent2D extent;
    VkRenderPass renderPass;
} Swapchain;

typedef struct {
    VkInstance instance;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice;
    QueueFamilyIndices indices;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkDebugUtilsMessengerEXT debugMessenger;

    Swapchain swapchain;

    VkCommandPool commandPool;

    /* frames */
    VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];
    VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];
    uint32_t currentFrame;

    int framebufferResized;
} VulkanContext;

int createInstance(VulkanContext* context, GLFWwindow* window, const char* appName, const char* engine, int debug);
void destroyInstance(VulkanContext* context);

#endif // !CORE_H
