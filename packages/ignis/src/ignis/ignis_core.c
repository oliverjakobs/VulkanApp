#include "ignis_core.h"

#include "swapchain.h"

typedef struct
{
    VkInstance instance;
    VkSurfaceKHR surface;

#ifdef IGNIS_DEBUG
    VkDebugUtilsMessengerEXT debugMessenger;
#endif

    VkPhysicalDevice physicalDevice;
    VkDevice device;

    uint32_t queueFamiliesSet;
    uint32_t queueFamilyIndices[IGNIS_QUEUE_FAMILY_MAX_ENUM];

    VkQueue queues[IGNIS_QUEUE_FAMILY_MAX_ENUM];

    IgnisSwapchain swapchain;

    uint16_t swapchainGeneration;
    uint16_t swapchainLastGeneration;

    VkCommandPool commandPool;
    VkCommandBuffer commandBuffers[IGNIS_MAX_FRAMES_IN_FLIGHT];

    uint32_t currentFrame;
    uint32_t imageIndex;

    // state
    VkViewport viewport;
    VkRect2D scissor;
    VkClearValue clearColor;
    VkClearValue depthStencil;
} IgnisContext;

#ifdef IGNIS_DEBUG

VKAPI_ATTR VkBool32 VKAPI_CALL ignisDebugUtilsMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data)
{
    switch (message_severity)
    {
        default:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            IGNIS_ERROR(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            IGNIS_WARN(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            IGNIS_INFO(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            IGNIS_TRACE(callback_data->pMessage);
            break;
    }
    return VK_FALSE;
}

#endif

static const char* const VALIDATION_LAYERS[] = {
    "VK_LAYER_KHRONOS_validation"
};

static const uint32_t VALIDATION_LAYER_COUNT = sizeof(VALIDATION_LAYERS) / sizeof(VALIDATION_LAYERS[0]);

static uint8_t ignisCheckValidationLayerSupport()
{
    uint32_t count;
    vkEnumerateInstanceLayerProperties(&count, NULL);
    if (!count || VALIDATION_LAYER_COUNT > count) return IGNIS_FAIL;

    VkLayerProperties* properties = ignisAlloc(sizeof(VkLayerProperties) * count);
    if (!properties) return IGNIS_FAIL;

    vkEnumerateInstanceLayerProperties(&count, properties);

    uint8_t found = IGNIS_FAIL;
    for (size_t i = 0; i < VALIDATION_LAYER_COUNT; ++i)
    {
        found = IGNIS_FAIL;
        for (size_t j = 0; j < count; ++j)
        {
            if (strcmp(VALIDATION_LAYERS[i], properties[j].layerName) == 0)
            {
                found = IGNIS_OK;
                break;
            }
        }

        if (!found) break;
    }

    ignisFree(properties, sizeof(VkLayerProperties) * count);
    return found;
}

static IgnisContext context;
static VkExtent2D cachedExtent;


uint8_t ignisPickPhysicalDevice();
uint8_t ignisCreateLogicalDevice();

uint8_t ignisCreateContext(const char* name, const IgnisPlatform* platform)
{
    const VkAllocationCallbacks* allocator = ignisGetAllocator();

#ifdef IGNIS_DEBUG
    if (!ignisCheckValidationLayerSupport())
    {
        IGNIS_ERROR("validation layers requested, but not available!");
        return IGNIS_FAIL;
    }
#endif

    // TODO: make platform agnostic
    const char* const extensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        "VK_KHR_win32_surface",
#ifdef IGNIS_DEBUG
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
    };

    const uint32_t extensionCount = sizeof(extensions) / sizeof(extensions[0]);
    // const char* const extensions = platform->queryExtensions(&extensionCount);

    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .apiVersion = VK_API_VERSION_1_2,
        .pApplicationName = name,
        .applicationVersion = VK_MAKE_API_VERSION(1,0,0,0),
        .pEngineName = "ignis",
        .engineVersion = VK_MAKE_API_VERSION(1,0,0,0)
    };

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = extensionCount,
        .ppEnabledExtensionNames = extensions,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = 0
    };

#ifdef IGNIS_DEBUG
    createInfo.enabledLayerCount = VALIDATION_LAYER_COUNT;
    createInfo.ppEnabledLayerNames = VALIDATION_LAYERS;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
                        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
                        // | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
                        // | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                    | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
                    | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
        .pfnUserCallback = ignisDebugUtilsMessengerCallback
    
    };
    createInfo.pNext = &debugCreateInfo;
#endif

    VkResult result = vkCreateInstance(&createInfo, allocator, &context.instance);
    if (result != VK_SUCCESS)
    {
        IGNIS_ERROR("vkCreateInstance failed with result: %u", result);
        return IGNIS_FAIL;
    }

    // Debugger
#ifdef IGNIS_DEBUG
    PFN_vkCreateDebugUtilsMessengerEXT func = IGNIS_VK_PFN(context.instance, vkCreateDebugUtilsMessengerEXT);
    if (func)
    {
        result = func(context.instance, &debugCreateInfo, allocator, &context.debugMessenger);
        if (result != VK_SUCCESS)
            IGNIS_WARN("Failed to create debug messenger with result: %u", result);
    }
    else
    {
        IGNIS_WARN("Could not find function 'vkCreateDebugUtilsMessengerEXT'");
    }
#endif

    // Surface
    result = platform->createSurface(context.instance, platform->context, allocator, &context.surface);
    if (result != VK_SUCCESS)
    {
        IGNIS_ERROR("Failed to create window surface with result: %u", result);
        return IGNIS_FAIL;
    }

    if (ignisPickPhysicalDevice())
    {
        IGNIS_ERROR("failed to pick physical device");
        return IGNIS_FAIL;
    }

    if (!ignisCreateLogicalDevice())
    {
        IGNIS_CRITICAL("failed to create logical device");
        return IGNIS_FAIL;
    }

    /* create command pool */
    VkCommandPoolCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = context.queueFamilyIndices[IGNIS_QUEUE_GRAPHICS]
    };

    if (vkCreateCommandPool(context.device, &info, allocator, &context.commandPool) != VK_SUCCESS)
    {
        IGNIS_ERROR("failed to create device command pool!");
        return IGNIS_FAIL;
    }

    /* allocate command buffers */
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = context.commandPool,
        .commandBufferCount = IGNIS_MAX_FRAMES_IN_FLIGHT
    };

    if (vkAllocateCommandBuffers(context.device, &allocInfo, context.commandBuffers) != VK_SUCCESS)
    {
        IGNIS_ERROR("Failed to allocate command buffers");
        return IGNIS_FAIL;
    }



    VkExtent2D extent = { 1280, 720 };
    if (!ignisCreateSwapchain(context.device, context.physicalDevice, context.surface, VK_NULL_HANDLE, extent, allocator, &context.swapchain))
    {
        IGNIS_CRITICAL("failed to create swapchain");
        return IGNIS_FAIL;
    }

    if (!ignisCreateSwapchainSyncObjects(context.device, allocator, &context.swapchain))
    {
        IGNIS_CRITICAL("failed to create swapchain sync objects");
        return IGNIS_FAIL;
    }

    context.currentFrame = 0;
    context.imageIndex = 0;

    context.swapchainGeneration = 0;
    context.swapchainLastGeneration = 0;

    // set default state
    ignisSetClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    ignisSetDepthStencil(1.0f, 0);
    ignisSetViewport(0.0f, 0.0f, context.swapchain.extent.width, context.swapchain.extent.height);
    ignisSetDepthRange(0.0f, 1.0f);
    ignisSetScissor(0, 0, context.swapchain.extent.width, context.swapchain.extent.height);

    return IGNIS_OK;
}

void ignisDestroyContext()
{
    const VkAllocationCallbacks* allocator = ignisGetAllocator();

    ignisDestroySwapchainSyncObjects(context.device, allocator, &context.swapchain);
    vkFreeCommandBuffers(context.device, context.commandPool, IGNIS_MAX_FRAMES_IN_FLIGHT, context.commandBuffers);
    vkDestroyCommandPool(context.device, context.commandPool, allocator);

    ignisDestroySwapchain(context.device, allocator, &context.swapchain);

    vkDestroyDevice(context.device, allocator);

    vkDestroySurfaceKHR(context.instance, context.surface, allocator);

#ifdef IGNIS_DEBUG

    PFN_vkDestroyDebugUtilsMessengerEXT func = IGNIS_VK_PFN(context.instance, vkDestroyDebugUtilsMessengerEXT);
    if (func)
        func(context.instance, context.debugMessenger, allocator);
    else
        IGNIS_WARN("Could not find function 'vkDestroyDebugUtilsMessengerEXT'");

#endif

    vkDestroyInstance(context.instance, allocator);
}



/* ---------------------------------| Device |------------------------------------------ */

// Device requirements
static const char* const REQ_EXTENSIONS[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static const uint32_t REQ_EXTENSION_COUNT = sizeof(REQ_EXTENSIONS) / sizeof(REQ_EXTENSIONS[0]);

static const uint32_t REQ_QUEUE_FAMILIES = IGNIS_QUEUE_GRAPHICS_BIT
                                        | IGNIS_QUEUE_TRANSFER_BIT
                                        | IGNIS_QUEUE_PRESENT_BIT;

static uint32_t ignisFindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface, uint32_t* indices);
static uint8_t ignisQuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
static uint8_t ignisCheckDeviceExtensionSupport(VkPhysicalDevice device);

uint8_t ignisPickPhysicalDevice()
{
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(context.instance, &count, NULL);
    if (!count) return IGNIS_FAIL;

    VkPhysicalDevice* devices = ignisAlloc(sizeof(VkPhysicalDevice) * count);
    if (!devices) return IGNIS_FAIL;

    vkEnumeratePhysicalDevices(context.instance, &count, devices);

    context.physicalDevice = VK_NULL_HANDLE;
    for (uint32_t i = 0; i < count; ++i)
    {
        // skip device if required queue families are not supported
        uint32_t familyIndices[IGNIS_QUEUE_FAMILY_MAX_ENUM];
        uint32_t familiesSet = ignisFindQueueFamilies(devices[i], context.surface, familyIndices);
        if (!(familiesSet & REQ_QUEUE_FAMILIES))
            continue;

        // skip device if required swapchain is not support
        if (!ignisQuerySwapChainSupport(devices[i], context.surface))
            continue;

        // skip device if required extensions are not supported
        if (!ignisCheckDeviceExtensionSupport(devices[i]))
            continue;

        // suitable device found
        context.physicalDevice = devices[i];
        context.queueFamiliesSet = familiesSet;
        memcpy(context.queueFamilyIndices, familyIndices, sizeof(uint32_t) * IGNIS_QUEUE_FAMILY_MAX_ENUM);
        break;
    }

    ignisFree(devices, sizeof(VkPhysicalDevice) * count);

    return context.physicalDevice == VK_NULL_HANDLE;
}

static uint8_t ignisArrayCheckUnique(uint32_t arr[], uint32_t size)
{
    for (uint32_t i = 1; i < size; ++i)
        if (arr[0] == arr[i]) return IGNIS_FAIL;
    return IGNIS_OK;
}

uint8_t ignisCreateLogicalDevice()
{
    uint32_t queueCount = 0;
    VkDeviceQueueCreateInfo queueCreateInfos[IGNIS_QUEUE_FAMILY_MAX_ENUM] = { 0 };

    float priority = 1.0f;
    for (uint32_t i = 0; i < IGNIS_QUEUE_FAMILY_MAX_ENUM; ++i)
    {
        if (!ignisArrayCheckUnique(context.queueFamilyIndices + i, IGNIS_QUEUE_FAMILY_MAX_ENUM - i))
            continue;

        queueCreateInfos[queueCount++] = (VkDeviceQueueCreateInfo){
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = context.queueFamilyIndices[i],
            .queueCount = 1,
            .pQueuePriorities = &priority,
        };
    }

    VkPhysicalDeviceFeatures deviceFeatures = { 0 };

    VkDeviceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = queueCreateInfos,
        .queueCreateInfoCount = queueCount,
        .pEnabledFeatures = &deviceFeatures,
        .ppEnabledExtensionNames = REQ_EXTENSIONS,
        .enabledExtensionCount = REQ_EXTENSION_COUNT
    };

    VkResult result = vkCreateDevice(context.physicalDevice, &createInfo, ignisGetAllocator(), &context.device);
    if (result != VK_SUCCESS)
        return IGNIS_FAIL;

    /* get queues */
    for (size_t i = 0; i < IGNIS_QUEUE_FAMILY_MAX_ENUM; ++i)
        vkGetDeviceQueue(context.device, context.queueFamilyIndices[i], 0, &context.queues[i]);

    return IGNIS_OK;
}

uint32_t ignisFindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface, uint32_t* indices)
{
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, NULL);
    if (!count) return 0;

    VkQueueFamilyProperties* properties = ignisAlloc(sizeof(VkQueueFamilyProperties) * count);
    if (!properties) return 0;

    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, properties);

    uint32_t familiesSet = 0;
    uint8_t minTransferScore = -1;
    for (uint32_t i = 0; i < count; ++i)
    {
        uint8_t currentTransferScore = 0;

        // graphics queue
        if (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices[IGNIS_QUEUE_GRAPHICS] = i;
            familiesSet |= IGNIS_QUEUE_GRAPHICS_BIT;
            ++currentTransferScore;
        }

        // compute queue
        if (properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            indices[IGNIS_QUEUE_COMPUTE] = i;
            familiesSet |= IGNIS_QUEUE_COMPUTE_BIT;
            ++currentTransferScore;
        }

        // transfer queue
        if (properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
        {
            // Take the index if it is the current lowest. This increases the
            // liklihood that it is a dedicated transfer queue.
            if (currentTransferScore <= minTransferScore)
            {
                minTransferScore = currentTransferScore;
                indices[IGNIS_QUEUE_TRANSFER] = i;
                familiesSet |= IGNIS_QUEUE_TRANSFER_BIT;
            }
        }

        VkBool32 supported = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supported);
        if (supported)
        {
            indices[IGNIS_QUEUE_PRESENT] = i;
            familiesSet |= IGNIS_QUEUE_PRESENT_BIT;
        }
    }

    ignisFree(properties, sizeof(VkQueueFamilyProperties) * count);
    return familiesSet;
}

uint8_t ignisQuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, NULL);

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, NULL);

    return formatCount > 0 && presentModeCount > 0;
}

uint8_t ignisCheckDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t count = 0;
    vkEnumerateDeviceExtensionProperties(device, NULL, &count, NULL);
    if (!count || REQ_EXTENSION_COUNT > count) return IGNIS_FAIL;

    VkExtensionProperties* properties = ignisAlloc(sizeof(VkExtensionProperties) * count);
    if (!properties) return IGNIS_FAIL;

    vkEnumerateDeviceExtensionProperties(device, NULL, &count, properties);

    uint8_t found = IGNIS_FAIL;
    for (size_t i = 0; i < REQ_EXTENSION_COUNT; ++i)
    {
        found = IGNIS_FAIL;
        for (size_t j = 0; j < count; ++j)
        {
            if (strcmp(REQ_EXTENSIONS[i], properties[j].extensionName) == 0)
            {
                found = IGNIS_OK;
                break;
            }
        }
        if (!found) break;
    }

    ignisFree(properties, sizeof(VkExtensionProperties) * count);
    return found;
}



VkDeviceMemory ignisAllocateDeviceMemory(VkMemoryRequirements requirements, VkMemoryPropertyFlags properties, const VkAllocationCallbacks* allocator)
{
    VkPhysicalDeviceMemoryProperties memoryProps;
    vkGetPhysicalDeviceMemoryProperties(context.physicalDevice, &memoryProps);

    uint32_t memoryTypeIndex = memoryProps.memoryTypeCount;
    for (uint32_t i = 0; i < memoryProps.memoryTypeCount; ++i)
    {
        if ((requirements.memoryTypeBits & (1 << i)) && (memoryProps.memoryTypes[i].propertyFlags & properties) == properties)
        {
            memoryTypeIndex = i;
            break;
        }
    }

    if (memoryTypeIndex == memoryProps.memoryTypeCount)
    {
        IGNIS_ERROR("failed to find suitable memory type!");
        return VK_NULL_HANDLE;
    }

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = requirements.size,
        .memoryTypeIndex = memoryTypeIndex
    };

    VkDeviceMemory memory = VK_NULL_HANDLE;
    if (vkAllocateMemory(context.device, &allocInfo, allocator, &memory) != VK_SUCCESS)
        IGNIS_ERROR("failed to find allocate device memory!");

    return memory;
}






uint8_t ignisResize(uint32_t width, uint32_t height)
{
    cachedExtent.width = width;
    cachedExtent.height = height;
    context.swapchainGeneration++;

    return IGNIS_OK;
}

void ignisSetClearColor(float r, float g, float b, float a)
{
    context.clearColor.color.float32[0] = r;
    context.clearColor.color.float32[1] = g;
    context.clearColor.color.float32[2] = b;
    context.clearColor.color.float32[3] = a;
}

void ignisSetDepthStencil(float depth, uint32_t stencil)
{
    context.depthStencil.depthStencil.depth = depth;
    context.depthStencil.depthStencil.stencil = stencil;
}

void ignisSetViewport(float x, float y, float width, float height)
{
    context.viewport.x = x;
    context.viewport.y = y;
    context.viewport.width = width;
    context.viewport.height = height;
}

void ignisSetDepthRange(float nearVal, float farVal)
{
    context.viewport.minDepth = nearVal;
    context.viewport.maxDepth = farVal;
}

void ignisSetScissor(int32_t x, int32_t y, uint32_t w, uint32_t h)
{
    context.scissor.offset.x = x;
    context.scissor.offset.y = y;
    context.scissor.extent.width = w;
    context.scissor.extent.height = h;
}

uint8_t ignisBeginFrame()
{
    // Check if swapchain is out of date.
    if (context.swapchainGeneration != context.swapchainLastGeneration)
    {
        // window is minimized
        if (cachedExtent.width == 0 || cachedExtent.height == 0)
            return IGNIS_FAIL;

        if (!ignisRecreateSwapchain(context.device, context.physicalDevice, context.surface, cachedExtent, ignisGetAllocator(), &context.swapchain))
        {
            IGNIS_ERROR("Failed to recreate swapchain");
            return IGNIS_FAIL;
        }
        context.swapchainLastGeneration = context.swapchainGeneration;

        IGNIS_TRACE("Recreated Swapchchain");
    }

    if (!ignisAcquireNextImage(context.device, &context.swapchain, context.currentFrame, &context.imageIndex))
        return IGNIS_FAIL;

    // Begin recording commands.
    VkCommandBuffer commandBuffer = context.commandBuffers[context.currentFrame];
    vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0
    };

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        IGNIS_WARN("failed to begin recording command buffer!");
        return IGNIS_FAIL;
    }

    // set dynamic state
    vkCmdSetViewport(commandBuffer, 0, 1, &context.viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &context.scissor);

    // begin render pass
    VkClearValue clearValues[] = {
        context.clearColor,
        context.depthStencil
    };

    VkRenderPassBeginInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = context.swapchain.renderPass,
        .framebuffer = context.swapchain.framebuffers[context.imageIndex],
        .renderArea.offset = { 0, 0 },
        .renderArea.extent = context.swapchain.extent,
        .pClearValues = clearValues,
        .clearValueCount = 2
    };

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    return IGNIS_OK;
}

uint8_t ignisEndFrame()
{
    VkCommandBuffer commandBuffer = context.commandBuffers[context.currentFrame];
    vkCmdEndRenderPass(commandBuffer);

    VkQueue graphicsQueue = context.queues[IGNIS_QUEUE_GRAPHICS];
    if (!ingisSubmitFrame(graphicsQueue, commandBuffer, context.currentFrame, &context.swapchain))
        IGNIS_WARN("failed to submit frame");

    VkQueue presentQueue = context.queues[IGNIS_QUEUE_PRESENT];
    if (!ignisPresentFrame(presentQueue, context.imageIndex, context.currentFrame, &context.swapchain))
        IGNIS_WARN("failed to present frame");
    
    /* next frame */
    context.currentFrame = (context.currentFrame + 1) % IGNIS_MAX_FRAMES_IN_FLIGHT;

    return IGNIS_OK;
}

VkInstance       ignisGetVkInstance()       { return context.instance; }
VkDevice         ignisGetVkDevice()         { return context.device; }
VkPhysicalDevice ignisGetVkPhysicalDevice() { return context.physicalDevice; }
VkRenderPass     ignisGetVkRenderPass()     { return context.swapchain.renderPass; }
VkCommandBuffer  ignisGetCommandBuffer()    { return context.commandBuffers[context.currentFrame]; }

uint32_t ignisGetCurrentFrame() { return context.currentFrame; }

uint32_t ignisGetQueueFamilyIndex(IgnisQueueFamily family) { return context.queueFamilyIndices[family]; }


const VkAllocationCallbacks* ignisGetAllocator() { return NULL; }


void ignisPrintInfo()
{
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(context.physicalDevice, &properties);

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(context.physicalDevice, &features);

    VkPhysicalDeviceMemoryProperties memory;
    vkGetPhysicalDeviceMemoryProperties(context.physicalDevice, &memory);

    IGNIS_INFO("Physical device: %s", properties.deviceName);
    
    const char* typeDesc[] = {
        [VK_PHYSICAL_DEVICE_TYPE_OTHER]          = "OTHER",
        [VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU] = "INTEGRATED GPU",
        [VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU]   = "DISCRETE GPU",
        [VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU]    = "VIRTUAL GPU",
        [VK_PHYSICAL_DEVICE_TYPE_CPU]            = "CPU",
    };
    IGNIS_INFO("  > Device Type: %s", typeDesc[properties.deviceType]);

    IGNIS_INFO("  > Driver Version: %d.%d.%d",
        VK_VERSION_MAJOR(properties.driverVersion),
        VK_VERSION_MINOR(properties.driverVersion),
        VK_VERSION_PATCH(properties.driverVersion));

    IGNIS_INFO("  > Vulkan API Version: %d.%d.%d",
        VK_VERSION_MAJOR(properties.apiVersion),
        VK_VERSION_MINOR(properties.apiVersion),
        VK_VERSION_PATCH(properties.apiVersion));

    IGNIS_INFO("  > Memory:");
    for (uint32_t i = 0; i < memory.memoryHeapCount; ++i)
    {
        float memory_size_gib = (((float)memory.memoryHeaps[i].size) / 1024.0f / 1024.0f / 1024.0f);

        if (memory.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
            IGNIS_INFO("    Local:  %.2f GiB", memory_size_gib);
        else
            IGNIS_INFO("    Shared: %.2f GiB", memory_size_gib);
    }

    
    IGNIS_INFO("Queues:");
    IGNIS_INFO("  > Graphics: %s (%d)", context.queueFamiliesSet & IGNIS_QUEUE_GRAPHICS_BIT ? "true" : "false", context.queueFamilyIndices[IGNIS_QUEUE_GRAPHICS]);
    IGNIS_INFO("  > Transfer: %s (%d)", context.queueFamiliesSet & IGNIS_QUEUE_TRANSFER_BIT ? "true" : "false", context.queueFamilyIndices[IGNIS_QUEUE_TRANSFER]);
    IGNIS_INFO("  > Compute:  %s (%d)", context.queueFamiliesSet & IGNIS_QUEUE_COMPUTE_BIT ? "true" : "false", context.queueFamilyIndices[IGNIS_QUEUE_COMPUTE]);
    IGNIS_INFO("  > Present:  %s (%d)", context.queueFamiliesSet & IGNIS_QUEUE_PRESENT_BIT ? "true" : "false", context.queueFamilyIndices[IGNIS_QUEUE_PRESENT]);
}