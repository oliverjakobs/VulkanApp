#ifndef PIPELINE_H
#define PIPELINE_H

#include "common.h"
#include "Core.h"

typedef struct {
    VkVertexInputBindingDescription* bindings;
    uint32_t bindingCount;
    VkVertexInputAttributeDescription* attributes;
    uint32_t attributeCount;
} VertexInputDescription;

typedef enum {
    SHADER_VERT = 0,
    SHADER_FRAG,
    SHADER_COUNT
} ShaderIndex;

typedef struct {
    VkPipeline handle;
    VkPipelineLayout layout;
    VertexInputDescription inputDesc;

    VkShaderModule shaderModules[SHADER_COUNT];
} Pipeline;

int createShaderStages(Pipeline* pipeline, const char* vertPath, const char* fragPath);
void destroyShaderStages(Pipeline* pipeline);

int createPipelineLayout(Pipeline* pipeline, VkDescriptorSetLayout descriptorSetLayout, const VertexInputDescription* desc);
void destroyPipelineLayout(Pipeline* pipeline);

int createPipeline(Pipeline* pipeline, const ObeliskSwapchain* swapchain);
int recreatePipeline(Pipeline* pipeline, const ObeliskSwapchain* swapchain);
void destroyPipeline(Pipeline* pipeline);


#endif // !PIPELINE_H
