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

int createShaderStages(const VulkanContext* context, Pipeline* pipeline, const char* vertPath, const char* fragPath);
void destroyShaderStages(const VulkanContext* context, Pipeline* pipeline);

int createPipelineLayout(const VulkanContext* context, Pipeline* pipeline, const VertexInputDescription* desc);
void destroyPipelineLayout(const VulkanContext* context, Pipeline* pipeline);

int createPipeline(const VulkanContext* context, Pipeline* pipeline);
int recreatePipeline(const VulkanContext* context, Pipeline* pipeline);
void destroyPipeline(const VulkanContext* context, Pipeline* pipeline);


#endif // !PIPELINE_H
