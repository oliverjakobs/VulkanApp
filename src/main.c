#include "Application.h"

#include "Device.h"
#include "Swapchain.h"
#include "Pipeline.h"
#include "Frame.h"

#include <string.h>

static VulkanContext context = { 0 };

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

VkResult CreateVulkanInstance(const char* appName, const char* engine) {
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

    VkResult result = vkCreateInstance(&createInfo, NULL, &context.instance);

    free(extensions);

    return result;
}

int OnLoad(MinimalApp* app, uint32_t w, uint32_t h) {
    if (CreateVulkanInstance("VulkanApp", "Ignis") != VK_SUCCESS) {
        MINIMAL_ERROR("Failed to create vulkan instance!");
        return MINIMAL_FAIL;
    }

    /* setup debug messenger */
    if (enableValidationLayers) {
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = { 0 };
        populateDebugMessengerCreateInfo(&debugCreateInfo, NULL);
        if (CreateDebugUtilsMessengerEXT(context.instance, &debugCreateInfo, NULL, &context.debugMessenger) != VK_SUCCESS) {
            MINIMAL_ERROR("failed to set up debug messenger!");
            return MINIMAL_FAIL;
        }
    }

    /* create surface */
    if (glfwCreateWindowSurface(context.instance, app->window, NULL, &context.surface) != VK_SUCCESS) {
        MINIMAL_ERROR("failed to create window surface!");
        return MINIMAL_FAIL;
    }

    /* pick physical device */
    QueueFamilyIndices indices;
    if (!pickPhysicalDevice(&context, &indices)) {
        MINIMAL_ERROR("failed to find a suitable GPU!");
        return MINIMAL_FAIL;
    }

    /* create logical device */
    if (!createLogicalDevice(&context, indices)) {
        MINIMAL_ERROR("failed to create logical device!");
        return MINIMAL_FAIL;
    }

    /* create swap chain */
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context.physicalDevice, context.surface, &capabilities);

    int width, height;
    glfwGetFramebufferSize(app->window, &width, &height);
    context.swapchain.extent = getSwapChainExtent(&capabilities, (uint32_t)width, (uint32_t)height);

    if (!createSwapChain(&context, &capabilities, indices)) {
        MINIMAL_ERROR("failed to create swap chain!");
        return MINIMAL_FAIL;
    }

    if (!createSwapChainImages(&context)) {
        MINIMAL_ERROR("failed to create swap chain images!");
        return MINIMAL_FAIL;
    }

    if (!createRenderPass(&context)) {
        MINIMAL_ERROR("failed to create render pass!");
        return MINIMAL_FAIL;
    }

    if (!createGraphicsPipeline(&context)) {
        MINIMAL_ERROR("failed to create graphics pipeline!");
        return MINIMAL_FAIL;
    }

    if (!createFramebuffers(&context)) {
        MINIMAL_ERROR("failed to create framebuffer!");
        return MINIMAL_FAIL;
    }

    if (!createCommandPool(&context, indices)) {
        MINIMAL_ERROR("failed to create command pool!");
        return MINIMAL_FAIL;
    }

    if (!createCommandBuffer(&context)) {
        MINIMAL_ERROR("failed to allocate command buffers!");
        return MINIMAL_FAIL;
    }

    if (!createSyncObjects(&context)) {
        MINIMAL_ERROR("failed to create synchronization objects for a frame!");
        return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
}

void OnDestroy(MinimalApp* app) {

    vkDeviceWaitIdle(context.device);

    destroySyncObjects(&context);

    vkDestroyCommandPool(context.device, context.commandPool, NULL);

    destroyFramebuffers(&context);

    /* destroy pipeline */
    vkDestroyPipeline(context.device, context.graphicsPipeline, NULL);
    vkDestroyRenderPass(context.device, context.renderPass, NULL);
    vkDestroyPipelineLayout(context.device, context.pipelineLayout, NULL);

    /* destroy swap chain */
    destroySwapChainImages(&context);
    vkDestroySwapchainKHR(context.device, context.swapchain.handle, NULL);

    /* destroy device */
    vkDestroyDevice(context.device, NULL);

    /* destroy debug messenger */
    if (enableValidationLayers) DestroyDebugUtilsMessengerEXT(context.instance, context.debugMessenger, NULL);

    /* destroy surface and instance */
    vkDestroySurfaceKHR(context.instance, context.surface, NULL);
    vkDestroyInstance(context.instance, NULL);
}

int OnEvent(MinimalApp* app, const MinimalEvent* e) {
    if (MinimalEventKeyPressed(e) == GLFW_KEY_ESCAPE) MinimalClose(app);
    return MINIMAL_OK;
}

void OnUpdate(MinimalApp* app, float deltatime) {
    vkWaitForFences(context.device, 1, &context.inFlightFences[context.currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(context.device, 1, &context.inFlightFences[context.currentFrame]);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(context.device, context.swapchain.handle, UINT64_MAX, context.imageAvailableSemaphores[context.currentFrame], VK_NULL_HANDLE, &imageIndex);

    vkResetCommandBuffer(context.commandBuffers[context.currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
    recordCommandBuffer(&context, context.commandBuffers[context.currentFrame], imageIndex);

    VkSubmitInfo submitInfo = { 0 };
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { context.imageAvailableSemaphores[context.currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.waitSemaphoreCount = 1;

    submitInfo.pCommandBuffers = &context.commandBuffers[context.currentFrame];
    submitInfo.commandBufferCount = 1;

    VkSemaphore signalSemaphores[] = { context.renderFinishedSemaphores[context.currentFrame] };
    submitInfo.pSignalSemaphores = signalSemaphores;
    submitInfo.signalSemaphoreCount = 1;

    if (vkQueueSubmit(context.graphicsQueue, 1, &submitInfo, context.inFlightFences[context.currentFrame]) != VK_SUCCESS) {
        MINIMAL_WARN("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo = { 0 };
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { context.swapchain.handle };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(context.presentQueue, &presentInfo);

    context.currentFrame = (context.currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
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