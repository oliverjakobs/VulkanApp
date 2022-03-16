#include "Application.h"

#include "Device.h"
#include "Swapchain.h"
#include "Pipeline.h"
#include "Frame.h"

#include <string.h>

const int enableValidationLayers = 1;

const char* const validationLayers[] = {
    "VK_LAYER_KHRONOS_validation"
};

const uint32_t validationLayerCount = sizeof(validationLayers) / sizeof(validationLayers[0]);

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data) {
    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        MINIMAL_ERROR("validation layer: %s", callback_data->pMessage);
    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* info, const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* messenger) {
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    return func ? func(instance, info, allocator, messenger) : VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* allocator) {
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func) func(instance, messenger, allocator);
}

char** getRequiredExtensions(uint32_t* count) {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    if (!glfwExtensionCount) return NULL;

    char** extensions = malloc(sizeof(char*) * ((size_t)glfwExtensionCount + 1));
    if (!extensions) return NULL;

    memcpy(extensions, glfwExtensions, sizeof(char*) * glfwExtensionCount);

    if (enableValidationLayers) extensions[glfwExtensionCount++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

    *count = glfwExtensionCount;
    return extensions;
}

int checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    if (!layerCount) return 0;

    VkLayerProperties* availableLayers = malloc(sizeof(VkLayerProperties) * layerCount);
    if (!availableLayers) return 0;

    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    int layerFound = 0;
    for (size_t i = 0; i < validationLayerCount; ++i) {
        layerFound = 0;
        const char* name = validationLayers[i];

        for (size_t available = 0; available < layerCount; ++available) {
            if (strcmp(name, availableLayers[available].layerName) == 0) {
                layerFound = 1;
                break;
            }
        }

        if (!layerFound) break;
    }

    free(availableLayers);
    return layerFound;
}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT* createInfo, void* userData) {
    createInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo->pfnUserCallback = debugCallback;
    createInfo->pUserData = userData;
}

VkResult CreateVulkanInstance(VulkanContext* context, const char* appName, const char* engine) {
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        MINIMAL_ERROR("validation layers requested, but not available!");
        return VK_ERROR_UNKNOWN;
    }

    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = appName,
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = engine,
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0
    };

    uint32_t extension_count = 0;
    char** extensions = getRequiredExtensions(&extension_count);

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .ppEnabledExtensionNames = extensions,
        .enabledExtensionCount = extension_count
    };

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = { 0 };
    populateDebugMessengerCreateInfo(&debugCreateInfo, NULL);
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = validationLayerCount;
        createInfo.ppEnabledLayerNames = validationLayers;
        createInfo.pNext = &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = NULL;
    }

    VkResult result = vkCreateInstance(&createInfo, NULL, &context->instance);

    free(extensions);

    return result;
}

int OnLoad(MinimalApp* app, uint32_t w, uint32_t h) {
    if (CreateVulkanInstance(&app->context, "VulkanApp", "Ignis") != VK_SUCCESS) {
        MINIMAL_ERROR("Failed to create vulkan instance!");
        return MINIMAL_FAIL;
    }

    /* setup debug messenger */
    if (enableValidationLayers) {
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = { 0 };
        populateDebugMessengerCreateInfo(&debugCreateInfo, NULL);
        if (CreateDebugUtilsMessengerEXT(app->context.instance, &debugCreateInfo, NULL, &app->context.debugMessenger) != VK_SUCCESS) {
            MINIMAL_ERROR("failed to set up debug messenger!");
            return MINIMAL_FAIL;
        }
    }

    /* create surface */
    if (glfwCreateWindowSurface(app->context.instance, app->window, NULL, &app->context.surface) != VK_SUCCESS) {
        MINIMAL_ERROR("failed to create window surface!");
        return MINIMAL_FAIL;
    }

    /* pick physical device */
    if (!pickPhysicalDevice(&app->context)) {
        MINIMAL_ERROR("failed to find a suitable GPU!");
        return MINIMAL_FAIL;
    }

    /* create logical device */
    if (!createLogicalDevice(&app->context)) {
        MINIMAL_ERROR("failed to create logical device!");
        return MINIMAL_FAIL;
    }

    /* create swap chain */
    int width, height;
    glfwGetFramebufferSize(app->window, &width, &height);

    if (!createSwapChain(&app->context, (uint32_t)width, (uint32_t)height)) {
        MINIMAL_ERROR("failed to create swap chain!");
        return MINIMAL_FAIL;
    }

    if (!createSwapChainImages(&app->context)) {
        MINIMAL_ERROR("failed to create swap chain images!");
        return MINIMAL_FAIL;
    }

    if (!createRenderPass(&app->context)) {
        MINIMAL_ERROR("failed to create render pass!");
        return MINIMAL_FAIL;
    }

    if (!createGraphicsPipeline(&app->context)) {
        MINIMAL_ERROR("failed to create graphics pipeline!");
        return MINIMAL_FAIL;
    }

    if (!createFramebuffers(&app->context)) {
        MINIMAL_ERROR("failed to create framebuffer!");
        return MINIMAL_FAIL;
    }

    if (!createCommandPool(&app->context)) {
        MINIMAL_ERROR("failed to create command pool!");
        return MINIMAL_FAIL;
    }

    if (!createCommandBuffer(&app->context)) {
        MINIMAL_ERROR("failed to allocate command buffers!");
        return MINIMAL_FAIL;
    }

    if (!createSyncObjects(&app->context)) {
        MINIMAL_ERROR("failed to create synchronization objects for a frame!");
        return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
}

void OnDestroy(MinimalApp* app) {
    destroySwapChain(&app->context);

    destroySyncObjects(&app->context);

    vkDestroyCommandPool(app->context.device, app->context.commandPool, NULL);

    /* destroy device */
    vkDestroyDevice(app->context.device, NULL);

    /* destroy debug messenger */
    if (enableValidationLayers) DestroyDebugUtilsMessengerEXT(app->context.instance, app->context.debugMessenger, NULL);

    /* destroy surface and instance */
    vkDestroySurfaceKHR(app->context.instance, app->context.surface, NULL);
    vkDestroyInstance(app->context.instance, NULL);
}

int OnEvent(MinimalApp* app, const MinimalEvent* e) {
    if (MinimalEventKeyPressed(e) == GLFW_KEY_ESCAPE) MinimalClose(app);
    return MINIMAL_OK;
}

void OnUpdate(MinimalApp* app, float deltatime) {
    vkWaitForFences(app->context.device, 1, &app->context.inFlightFences[app->context.currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(app->context.device, app->context.swapchain.handle, UINT64_MAX, app->context.imageAvailableSemaphores[app->context.currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain(&app->context, app->window);
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        MINIMAL_ERROR("failed to acquire swap chain image!");
        return;
    }

    vkResetFences(app->context.device, 1, &app->context.inFlightFences[app->context.currentFrame]);

    vkResetCommandBuffer(app->context.commandBuffers[app->context.currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
    recordCommandBuffer(&app->context, app->context.commandBuffers[app->context.currentFrame], imageIndex);

    VkSubmitInfo submitInfo = { 0 };
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { app->context.imageAvailableSemaphores[app->context.currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.waitSemaphoreCount = 1;

    submitInfo.pCommandBuffers = &app->context.commandBuffers[app->context.currentFrame];
    submitInfo.commandBufferCount = 1;

    VkSemaphore signalSemaphores[] = { app->context.renderFinishedSemaphores[app->context.currentFrame] };
    submitInfo.pSignalSemaphores = signalSemaphores;
    submitInfo.signalSemaphoreCount = 1;

    if (vkQueueSubmit(app->context.graphicsQueue, 1, &submitInfo, app->context.inFlightFences[app->context.currentFrame]) != VK_SUCCESS) {
        MINIMAL_WARN("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo = { 0 };
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { app->context.swapchain.handle };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(app->context.presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || app->context.framebufferResized) {
        app->context.framebufferResized = 0;
        recreateSwapChain(&app->context, app->window);
    } else if (result != VK_SUCCESS) {
        MINIMAL_ERROR("failed to present swap chain image!");
        return;
    }

    app->context.currentFrame = (app->context.currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

int main() {
    MinimalApp app = {
        .on_load = OnLoad,
        .on_destroy = OnDestroy,
        .on_event = OnEvent,
        .on_update = OnUpdate
    };

    if (MinimalLoad(&app, "VulkanApp", 1024, 800))
        MinimalRun(&app);

    MinimalDestroy(&app);

    return 0;
}