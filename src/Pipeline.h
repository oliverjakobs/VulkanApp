#ifndef PIPELINE_H
#define PIPELINE_H

#include "common.h"

typedef enum {
    SHADER_VERT = 0,
    SHADER_FRAG,
    SHADER_COUNT
} ShaderIndex;

typedef struct {
    VkPipeline handle;
    VkPipelineLayout layout;

    VkPipelineShaderStageCreateInfo shaderStages[SHADER_COUNT];
} Pipeline;

int pipelineCreateShaderStages(const VulkanContext* context, Pipeline* pipeline, const char* vertPath, const char* fragPath);
void pipelineDestroyShaderStages(const VulkanContext* context, Pipeline* pipeline);

int pipelineCreate(const VulkanContext* context, Pipeline* pipeline);
int pipelineRecreate(const VulkanContext* context, Pipeline* pipeline);
void pipelineDestroy(const VulkanContext* context, Pipeline* pipeline);


#endif // !PIPELINE_H
