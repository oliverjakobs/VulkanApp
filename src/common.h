#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>

#define MINIMAL_FAIL    0
#define MINIMAL_OK      1

#ifndef _DEBUG
#define MINIMAL_DISABLE_LOGGING
#define MINIMAL_DISABLE_ASSERT
#endif

/* minimal version numbers */
#define MINIMAL_VERSION_MAJOR       1
#define MINIMAL_VERSION_MINOR       3
#define MINIMAL_VERSION_REVISION    3

void MinimalGetVersion(int* major, int* minor, int* rev);
const char* MinimalGetVersionString();

typedef struct MinimalEvent MinimalEvent;
typedef struct MinimalApp MinimalApp;

/* --------------------------| logging |--------------------------------- */
#ifndef MINIMAL_DISABLE_LOGGING

#define MINIMAL_TRACE(s, ...)     MinimalLoggerPrint(stdout, MINIMAL_LOG_TRACE, s, __VA_ARGS__)
#define MINIMAL_INFO(s, ...)      MinimalLoggerPrint(stdout, MINIMAL_LOG_INFO, s, __VA_ARGS__)
#define MINIMAL_WARN(s, ...)      MinimalLoggerPrint(stdout, MINIMAL_LOG_WARN, s, __VA_ARGS__)
#define MINIMAL_ERROR(s, ...)     MinimalLoggerPrint(stdout, MINIMAL_LOG_ERROR, s, __VA_ARGS__)
#define MINIMAL_CRITICAL(s, ...)  MinimalLoggerPrint(stdout, MINIMAL_LOG_CRITICAL, s, __VA_ARGS__)

#else

#define MINIMAL_TRACE(s, ...)
#define MINIMAL_INFO(s, ...)
#define MINIMAL_WARN(s, ...)
#define MINIMAL_ERROR(s, ...)
#define MINIMAL_CRITICAL(s, ...)

#endif

typedef enum {
    MINIMAL_LOG_TRACE,
    MINIMAL_LOG_INFO,
    MINIMAL_LOG_WARN,
    MINIMAL_LOG_ERROR,
    MINIMAL_LOG_CRITICAL
} MinimalLogLevel;

void MinimalLoggerPrint(FILE* const stream, MinimalLogLevel level, const char* fmt, ...);
void MinimalLoggerPrintV(FILE* const stream, MinimalLogLevel level, const char* fmt, va_list args);


/* --------------------------| assert |---------------------------------- */
#ifndef MINIMAL_DISABLE_ASSERT

#include <assert.h>

#define MINIMAL_ASSERT(expr, msg) assert(((void)(msg), (expr)))

#else

#define MINIMAL_ASSERT(expr, msg) 

#endif

/* --------------------------| vulkan |---------------------------------- */

#define MAX_FRAMES_IN_FLIGHT 2

typedef struct {
    uint32_t familiesSet;
    uint32_t graphicsFamily;
    uint32_t presentFamily;
} QueueFamilyIndices;

typedef struct {
    VkInstance instance;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice;
    QueueFamilyIndices indices;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkDebugUtilsMessengerEXT debugMessenger;

    /* swapchain */
    struct {
        VkSwapchainKHR handle;
        VkImage* images;
        VkImageView* views;
        VkFramebuffer* framebuffers;
        uint32_t count;
        VkFormat format;
        VkExtent2D extent;
        VkRenderPass renderPass;
    } swapchain;

    /* frames */
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];
    VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];
    uint32_t currentFrame;

    int framebufferResized;
} VulkanContext;


/* --------------------------| utils |----------------------------------- */
uint32_t clamp32(uint32_t val, uint32_t min, uint32_t max);
char* readSPIRV(const char* path, size_t* sizeptr);


#endif // !COMMON_H
