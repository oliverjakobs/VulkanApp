#ifndef OBELISK_PIPELINE_H
#define OBELISK_PIPELINE_H

#include "core/core.h"

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
    OBELISK_SHADER_VERT = 0,
    OBELISK_SHADER_FRAG,
    OBELISK_SHADER_COUNT
} ObeliskShaderIndex;

typedef struct {
    VkPipeline handle;

    VkPipelineLayout layout;
    const ObeliskVertexLayout* vertexLayout;

    VkShaderModule shaderModules[OBELISK_SHADER_COUNT];
} ObeliskPipeline;

int obeliskCreateShaderStages(ObeliskPipeline* pipeline, const char* vertPath, const char* fragPath);
void obeliskDestroyShaderStages(ObeliskPipeline* pipeline);

int obeliskCreatePipelineLayout(ObeliskPipeline* pipeline, VkDescriptorSetLayout setLayout, uint32_t pushConstantSize);
void obeliskDestroyPipelineLayout(ObeliskPipeline* pipeline);

int obeliskCreatePipeline(ObeliskPipeline* pipeline, VkRenderPass renderPass, const ObeliskVertexLayout* vertexLayout);
int obeliskRecreatePipeline(ObeliskPipeline* pipeline, VkRenderPass renderPass);
void obeliskDestroyPipeline(ObeliskPipeline* pipeline);

#endif // !OBELISK_PIPELINE_H
