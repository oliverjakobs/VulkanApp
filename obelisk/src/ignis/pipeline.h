#ifndef IGNIS_PIPELINE_H
#define IGNIS_PIPELINE_H

#include "ignis_core.h"

typedef struct
{
    const char* vertPath;
    const char* fragPath;

    VkRenderPass renderPass;
} IgnisPipelineConfig;


typedef struct
{
    VkPipeline handle;
    VkPipelineLayout layout;
} IgnisPipeline;

uint8_t ignisCreatePipeline(VkDevice device, const IgnisPipelineConfig* config, IgnisPipeline* pipeline);
void ignisDestroyPipeline(VkDevice device, IgnisPipeline* pipeline);

#endif /* !IGNIS_PIPELINE_H */