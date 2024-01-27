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

uint8_t ignisCreatePipeline(const IgnisPipelineConfig* config, IgnisPipeline* pipeline);
void ignisDestroyPipeline(IgnisPipeline* pipeline);

#endif /* !IGNIS_PIPELINE_H */