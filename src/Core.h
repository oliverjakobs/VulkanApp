#ifndef CORE_H
#define CORE_H

#include "common.h"
#include "Buffer.h"
#include "Swapchain.h"

typedef struct obeliskContext obelistContext;

typedef struct {
    uint32_t familiesSet;
    uint32_t graphicsFamily;
    uint32_t presentFamily;
} QueueFamilyIndices;

struct VulkanContext {
    VkInstance instance;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice;
    QueueFamilyIndices indices;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    Swapchain swapchain;

    VkCommandPool commandPool;
    VkDescriptorPool descriptorPool;

    VkDescriptorSetLayout descriptorSetLayout;

    /* frames */
    VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
    VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
    Buffer uniformBuffers[MAX_FRAMES_IN_FLIGHT];
};

int createInstance(VulkanContext* context, GLFWwindow* window, const char* appName, const char* engine, int debug);
void destroyInstance(VulkanContext* context);

int createCommandPool(VulkanContext* context);

/* */
int obeliskCreateContext(GLFWwindow* window, const char* app, const char* engine, int debug);
void obeliskDestroyContext();

VkDevice obeliskGetContextDevice();
VkPhysicalDevice obeliskGetContextPhysicalDevice();

void obeliskPrintInfo();

#endif // !CORE_H
