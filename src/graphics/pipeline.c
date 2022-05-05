#include "pipeline.h"

#include "../utility/file.h"

static int obeliskCreateShaderModuleSrc(VkShaderModule* module, const uint32_t* code, size_t size) {
    VkShaderModuleCreateInfo info = { 
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = size,
        .pCode = (const uint32_t*)code
    };

    if (vkCreateShaderModule(obeliskGetDevice(), &info, NULL, module) != VK_SUCCESS) {
        return OBELISK_FAIL;
    }

    return OBELISK_OK;
}

static int obeliskCreateShaderModuleSPIRV(VkShaderModule* module, const char* path) {
    size_t size = 0;
    char* code = obeliskReadFile(path, &size);
    if (!code) return OBELISK_FAIL;

    int result = obeliskCreateShaderModuleSrc(module, (const uint32_t*)code, size);
    free(code);
    return result;
}

int obeliskCreateShaderStages(ObeliskPipeline* pipeline, const char* vertPath, const char* fragPath) {
    if (!obeliskCreateShaderModuleSPIRV(&pipeline->shaderModules[OBELISK_SHADER_VERT], vertPath)) {
        OBELISK_ERROR("failed to create shader module for %s", vertPath);
        return OBELISK_FAIL;
    }

    if (!obeliskCreateShaderModuleSPIRV(&pipeline->shaderModules[OBELISK_SHADER_FRAG], fragPath)) {
        OBELISK_ERROR("failed to create shader module for %s", fragPath);
        return OBELISK_FAIL;
    }

    return OBELISK_OK;
}

void obeliskDestroyShaderStages(ObeliskPipeline* pipeline) {
    for (size_t i = 0; i < OBELISK_SHADER_COUNT; ++i) {
        vkDestroyShaderModule(obeliskGetDevice(), pipeline->shaderModules[i], NULL);
    }
}

int obeliskCreatePipelineLayout(ObeliskPipeline* pipeline, VkDescriptorSetLayout setLayout, uint32_t pushConstantSize) {
    VkPushConstantRange pushConstantRange = {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = pushConstantSize
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pSetLayouts = &setLayout,
        .setLayoutCount = 1,
        .pPushConstantRanges = &pushConstantRange,
        .pushConstantRangeCount = 1
    };

    if (vkCreatePipelineLayout(obeliskGetDevice(), &pipelineLayoutInfo, NULL, &pipeline->layout) != VK_SUCCESS) {
        return OBELISK_FAIL;
    }

    return OBELISK_OK;
}

void obeliskDestroyPipelineLayout(ObeliskPipeline* pipeline) {
    vkDestroyPipelineLayout(obeliskGetDevice(), pipeline->layout, NULL);
}

int obeliskCreatePipeline(ObeliskPipeline* pipeline, VkRenderPass renderPass, const ObeliskVertexLayout* vertexLayout) {
    pipeline->vertexLayout = vertexLayout;

    /* shader stages */
    VkPipelineShaderStageCreateInfo shaderStages[OBELISK_SHADER_COUNT];
    shaderStages[OBELISK_SHADER_VERT] = (VkPipelineShaderStageCreateInfo){
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = pipeline->shaderModules[OBELISK_SHADER_VERT],
        .pName = "main"
    };

    shaderStages[OBELISK_SHADER_FRAG] = (VkPipelineShaderStageCreateInfo){
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = pipeline->shaderModules[OBELISK_SHADER_FRAG],
        .pName = "main"
    };

    /* vertex input state */
    VkVertexInputBindingDescription vertexBindingDesc = {
        .binding = 0,
        .stride = pipeline->vertexLayout->stride,
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pVertexBindingDescriptions = &vertexBindingDesc,
        .vertexBindingDescriptionCount = 1,
        .pVertexAttributeDescriptions = pipeline->vertexLayout->attributes,
        .vertexAttributeDescriptionCount = pipeline->vertexLayout->attributeCount
    };

    /* input assembly state */
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    /* viewport state */
    VkPipelineViewportStateCreateInfo viewportStateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pViewports = NULL,
        .viewportCount = 1,
        .pScissors = NULL,
        .scissorCount = 1
    };

    /* rasterization state  */
    VkPipelineRasterizationStateCreateInfo rasterizationInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f
    };

    /* multisample state */
    VkPipelineMultisampleStateCreateInfo multisampleInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .minSampleShading = 1.0f,
        .pSampleMask = NULL,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE
    };

    /* depth stencil state */
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .minDepthBounds = 0.0f,  // Optional
        .maxDepthBounds = 1.0f,  // Optional
        .stencilTestEnable = VK_FALSE,
        .front = { 0 },  // Optional
        .back = { 0 }   // Optional
    };

    /* color blend state  */
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT
                        | VK_COLOR_COMPONENT_G_BIT
                        | VK_COLOR_COMPONENT_B_BIT
                        | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD
    };

    VkPipelineColorBlendStateCreateInfo colorBlendInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .pAttachments = &colorBlendAttachment,
        .attachmentCount = 1,
        .blendConstants[0] = 0.0f,
        .blendConstants[1] = 0.0f,
        .blendConstants[2] = 0.0f,
        .blendConstants[3] = 0.0f
    };

    /* dynamic state */
    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicStateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pDynamicStates = dynamicStates,
        .dynamicStateCount = sizeof(dynamicStates) / sizeof(VkDynamicState),
        .flags = 0
    };

    /* create pipeline */
    VkGraphicsPipelineCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pStages = shaderStages,
        .stageCount = OBELISK_SHADER_COUNT,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssemblyInfo,
        .pViewportState = &viewportStateInfo,
        .pRasterizationState = &rasterizationInfo,
        .pMultisampleState = &multisampleInfo,
        .pDepthStencilState = &depthStencilInfo,
        .pColorBlendState = &colorBlendInfo,
        .pDynamicState = &dynamicStateInfo,
        .layout = pipeline->layout,
        .renderPass = renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1
    };

    if (vkCreateGraphicsPipelines(obeliskGetDevice(), VK_NULL_HANDLE, 1, &info, NULL, &pipeline->handle) != VK_SUCCESS) {
        return OBELISK_FAIL;
    }

    return OBELISK_OK;
}

int obeliskRecreatePipeline(ObeliskPipeline* pipeline, VkRenderPass renderPass) {
    obeliskDestroyPipeline(pipeline);

    if (!obeliskCreatePipeline(pipeline, renderPass, pipeline->vertexLayout)) {
        OBELISK_ERROR("failed to recreate pipeline!");
        return OBELISK_FAIL;
    }

    return OBELISK_OK;
}

void obeliskDestroyPipeline(ObeliskPipeline* pipeline) {
    vkDestroyPipeline(obeliskGetDevice(), pipeline->handle, NULL);
}
