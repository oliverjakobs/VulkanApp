#ifndef IGNIS_PIPELINE_H
#define IGNIS_PIPELINE_H

#include "ignis_core.h"

#include "buffer.h"
#include "texture.h"
#include "swapchain.h"

VkShaderModule ignisCreateShaderModule(const char* path);
void ignisDestroyShaderModule(VkShaderModule shader);

typedef struct
{
    VkVertexInputAttributeDescription* vertexAttributes;
    size_t attributeCount;
    uint32_t vertexStride;

    uint32_t uniformBufferSize;

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

uint8_t ignisCreatePipeline(const IgnisPipelineConfig* config, VkShaderModule vert, VkShaderModule frag, IgnisPipeline* pipeline);
void ignisDestroyPipeline(IgnisPipeline* pipeline);

void ignisBindPipeline(VkCommandBuffer commandBuffer, IgnisPipeline* pipeline);

uint8_t ignisPushUniform(IgnisPipeline* pipeline, const void* data, uint32_t size, uint32_t offset);

uint8_t ignisBindTexture(IgnisPipeline* pipeline, const IgnisTexture* texture, uint32_t binding);

#endif /* !IGNIS_PIPELINE_H */