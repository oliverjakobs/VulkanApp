#ifndef IGNIS_PIPELINE_H
#define IGNIS_PIPELINE_H

#include "ignis_core.h"

#include "buffer.h"
#include "texture.h"
#include "swapchain.h"

typedef struct
{
    float model[4][4];
    float view[4][4];
    float proj[4][4];
} UniformBufferObject;


typedef struct
{
    const char* vertPath;
    const char* fragPath;

    VkVertexInputAttributeDescription* vertexAttributes;
    size_t attributeCount;
    uint32_t vertexStride;

    /* rasterizer */
    VkCullModeFlags cullMode;
    VkFrontFace     frontFace;
} IgnisPipelineConfig;


typedef struct
{
    VkPipeline handle;
    VkPipelineLayout layout;
    
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;

    VkDescriptorSet descriptorSets[IGNIS_MAX_FRAMES_IN_FLIGHT];

    IgnisBuffer uniformBuffers[IGNIS_MAX_FRAMES_IN_FLIGHT];
    void*       uniformBufferData[IGNIS_MAX_FRAMES_IN_FLIGHT];
    uint32_t    uniformBufferSize;
} IgnisPipeline;

uint8_t ignisCreatePipeline(const IgnisPipelineConfig* config, IgnisPipeline* pipeline);
void ignisDestroyPipeline(IgnisPipeline* pipeline);

void ignisBindPipeline(VkCommandBuffer commandBuffer, IgnisPipeline* pipeline);

uint8_t ignisPipelinePushUniform(IgnisPipeline* pipeline, const void* data, uint32_t size, uint32_t offset);

uint8_t ignisPipelineBindTexture(IgnisPipeline* pipeline, const IgnisTexture* texture, uint32_t binding);

#endif /* !IGNIS_PIPELINE_H */