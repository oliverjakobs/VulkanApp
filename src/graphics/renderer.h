#ifndef OBELISK_RENDERER_H
#define OBELISK_RENDERER_H

#include "swapchain.h"
#include "buffer.h"

#include "cglm/cglm.h"

typedef struct {
    mat4 view;
    mat4 proj;
} UniformBufferObject;

typedef struct {
    ObeliskSwapchain swapchain;
    uint32_t frame;
    uint32_t imageIndex;

    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;

    VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
    VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
    ObeliskBuffer uniformBuffers[MAX_FRAMES_IN_FLIGHT];
} ObeliskRenderer;

int obeliskCreateRenderer(ObeliskRenderer* renderer, uint32_t width, uint32_t height);
void obeliskDestroyRenderer(ObeliskRenderer* renderer);

int obeliskCreateDescriptorPool(ObeliskRenderer* renderer);
void obeliskDestroyDescriptorPool(ObeliskRenderer* renderer);

int obeliskCreateDescriptorLayout(ObeliskRenderer* renderer);
void obeliskDestroyDescriptorLayout(ObeliskRenderer* renderer);

int obeliskCreateDescriptorSets(ObeliskRenderer* renderer);

VkCommandBuffer obeliskBeginFrame(ObeliskRenderer* renderer);
void obeliskEndFrame(ObeliskRenderer* renderer);

void obeliskBeginRenderPass(ObeliskRenderer* renderer, VkCommandBuffer cmdBuffer);
void obeliskEndRenderPass(ObeliskRenderer* renderer, VkCommandBuffer cmdBuffer);

void obeliskWriteUniform(ObeliskRenderer* renderer, UniformBufferObject* ubo);

float obeliskGetRendererAspect(const ObeliskRenderer* renderer);

#endif // !OBELISK_RENDERER_H
