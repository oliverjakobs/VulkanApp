#ifndef PIPELINE_H
#define PIPELINE_H

#include "common.h"
#include "Core.h"


typedef struct {
    VkFormat format;
    uint32_t offset;
} ObeliskVertexAttribute;

typedef struct {
    uint32_t stride;
    const VkVertexInputAttributeDescription* attributes;
    uint32_t attributeCount;
} ObeliskVertexLayout;

typedef enum {
    SHADER_VERT = 0,
    SHADER_FRAG,
    SHADER_COUNT
} ShaderIndex;

typedef struct {
    VkPipeline handle;

    VkPipelineLayout layout;
    const ObeliskVertexLayout* vertexLayout;

    VkShaderModule shaderModules[SHADER_COUNT];
} ObeliskPipeline;

int createShaderStages(ObeliskPipeline* pipeline, const char* vertPath, const char* fragPath);
void destroyShaderStages(ObeliskPipeline* pipeline);

int createPipelineLayout(ObeliskPipeline* pipeline, VkDescriptorSetLayout setLayout, const ObeliskVertexLayout* vertexLayout);
void destroyPipelineLayout(ObeliskPipeline* pipeline);

int createPipeline(ObeliskPipeline* pipeline, VkRenderPass renderPass);
int recreatePipeline(ObeliskPipeline* pipeline, VkRenderPass renderPass);
void destroyPipeline(ObeliskPipeline* pipeline);


#endif // !PIPELINE_H
