#ifndef CORE_H
#define CORE_H

#include "common.h"
#include "Buffer.h"
#include "Swapchain.h"

typedef struct obeliskContext obeliskContext;

int obeliskCreateInstance(obeliskContext* context, const char* app, const char* engine, int debug);
int obeliskPickPhysicalDevice(obeliskContext* context);
int obeliskCreateLogicalDevice(obeliskContext* context);

/* */
int obeliskCreateContext(GLFWwindow* window, const char* app, int debug);
void obeliskDestroyContext();

VkDevice obeliskGetDevice();
VkPhysicalDevice obeliskGetPhysicalDevice();
VkSurfaceKHR obeliskGetSurface();
VkQueue obeliskGetGraphicsQueue();
VkQueue obeliskGetPresentQueue();
uint32_t obeliskGetQueueGraphicsFamilyIndex();
uint32_t obeliskGetQueuePresentFamilyIndex();

VkResult obeliskGetPhysicalDeviceSurfaceCapabilities(VkSurfaceCapabilitiesKHR* capabilities);

void obeliskPrintInfo();

VkResult obeliskAllocateCommandBuffers(VkCommandBuffer* buffers, VkCommandBufferLevel level, uint32_t count);
void obeliskFreeCommandBuffers(const VkCommandBuffer* buffers, uint32_t count);

#endif // !CORE_H
