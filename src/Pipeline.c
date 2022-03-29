#include "Pipeline.h"

#include "Buffer.h"

static int createShaderModuleSrc(VkShaderModule* module, const uint32_t* code, size_t size) {
    VkShaderModuleCreateInfo info = { 
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = size,
        .pCode = (const uint32_t*)code
    };

    if (vkCreateShaderModule(obeliskGetDevice(), &info, NULL, module) != VK_SUCCESS) {
        return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
}

static int createShaderModuleSPIRV(VkShaderModule* module, const char* path) {
    size_t size = 0;
    char* code = readFile(path, &size);
    if (!code) return MINIMAL_FAIL;

    int result = createShaderModuleSrc(module, (const uint32_t*)code, size);
    free(code);
    return result;
}

int createShaderStages(Pipeline* pipeline, const char* vertPath, const char* fragPath) {
    if (!createShaderModuleSPIRV(&pipeline->shaderModules[SHADER_VERT], vertPath)) {
        MINIMAL_ERROR("failed to create shader module for %s", vertPath);
        return MINIMAL_FAIL;
    }

    if (!createShaderModuleSPIRV(&pipeline->shaderModules[SHADER_FRAG], fragPath)) {
        MINIMAL_ERROR("failed to create shader module for %s", fragPath);
        return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
}

void destroyShaderStages(Pipeline* pipeline) {
    for (size_t i = 0; i < SHADER_COUNT; ++i) {
        vkDestroyShaderModule(obeliskGetDevice(), pipeline->shaderModules[i], NULL);
    }
}

int createPipelineLayout(Pipeline* pipeline, const ObeliskSwapchain* swapchain, const ObeliskPipelineVertexLayout* layout) {

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pSetLayouts = &swapchain->descriptorSetLayout,
        .setLayoutCount = 1,
        .pPushConstantRanges = NULL,
        .pushConstantRangeCount = 0
    };

    if (vkCreatePipelineLayout(obeliskGetDevice(), &pipelineLayoutInfo, NULL, &pipeline->layout) != VK_SUCCESS) {
        return MINIMAL_FAIL;
    }

    pipeline->vertexInputInfo = (VkPipelineVertexInputStateCreateInfo){
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pVertexBindingDescriptions = layout->vertexInputBindings,
        .vertexBindingDescriptionCount = layout->vertexInputBindingCount,
        .pVertexAttributeDescriptions = layout->vertexInputAttributes,
        .vertexAttributeDescriptionCount = layout->vertexInputAttributeCount
    };

    return MINIMAL_OK;
}

void destroyPipelineLayout(Pipeline* pipeline) {
    vkDestroyPipelineLayout(obeliskGetDevice(), pipeline->layout, NULL);
}

int createPipeline(Pipeline* pipeline, VkRenderPass renderPass) {
    /* shader stages */
    VkPipelineShaderStageCreateInfo shaderStages[SHADER_COUNT];
    shaderStages[SHADER_VERT] = (VkPipelineShaderStageCreateInfo){
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = pipeline->shaderModules[SHADER_VERT],
        .pName = "main"
    };

    shaderStages[SHADER_FRAG] = (VkPipelineShaderStageCreateInfo){
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = pipeline->shaderModules[SHADER_FRAG],
        .pName = "main"
    };

    /* vertex input state */

    /* input assembly state */
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    /* viewport state */
    VkPipelineViewportStateCreateInfo viewportState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pViewports = NULL,
        .viewportCount = 1,
        .pScissors = NULL,
        .scissorCount = 1
    };

    /* rasterization state  */
    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f
    };

    /* multisample state */
    VkPipelineMultisampleStateCreateInfo multisampling = {
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

    VkPipelineColorBlendStateCreateInfo colorBlending = {
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
        .stageCount = SHADER_COUNT,
        .pVertexInputState = &pipeline->vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = &depthStencilInfo,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicStateInfo,
        .layout = pipeline->layout,
        .renderPass = renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1
    };

    if (vkCreateGraphicsPipelines(obeliskGetDevice(), VK_NULL_HANDLE, 1, &info, NULL, &pipeline->handle) != VK_SUCCESS) {
        return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
}

int recreatePipeline(Pipeline* pipeline, VkRenderPass renderPass) {
    destroyPipeline(pipeline);

    if (!createPipeline(pipeline, renderPass)) {
        MINIMAL_ERROR("failed to recreate pipeline!");
        return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
}

void destroyPipeline(Pipeline* pipeline) {
    vkDestroyPipeline(obeliskGetDevice(), pipeline->handle, NULL);
}
