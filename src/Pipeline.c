#include "Pipeline.h"

#include "Buffer.h"

static VkShaderModule createShaderModule(VulkanContext* context, VkShaderModule* module, const char* path) {
    size_t size = 0;
    char* code = readSPIRV(path, &size);

    if (!code) return MINIMAL_FAIL;

    VkShaderModuleCreateInfo info = { 
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = size,
        .pCode = (const uint32_t*)code
    };

    VkResult result = vkCreateShaderModule(context->device, &info, NULL, module);

    free(code);

    if (result != VK_SUCCESS) {
        return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
}

int pipelineCreateShaderStages(const VulkanContext* context, Pipeline* pipeline, const char* vertPath, const char* fragPath) {
    VkShaderModule vert;
    if (!createShaderModule(context, &vert, vertPath)) {
        MINIMAL_ERROR("failed to create shader module for %s", vertPath);
        return MINIMAL_FAIL;
    }

    VkShaderModule frag;
    if (!createShaderModule(context, &frag, fragPath)) {
        MINIMAL_ERROR("failed to create shader module for %s", fragPath);
        return MINIMAL_FAIL;
    }

    pipeline->shaderStages[SHADER_VERT] = (VkPipelineShaderStageCreateInfo){
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vert,
        .pName = "main"
    };

    pipeline->shaderStages[SHADER_FRAG] = (VkPipelineShaderStageCreateInfo){
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = frag,
        .pName = "main"
    };

    return MINIMAL_OK;
}

void pipelineDestroyShaderStages(const VulkanContext* context, Pipeline* pipeline) {
    for (size_t i = 0; i < SHADER_COUNT; ++i) {
        vkDestroyShaderModule(context->device, pipeline->shaderStages[i].module, NULL);
    }
}

int pipelineCreate(const VulkanContext* context, Pipeline* pipeline) {
    /* create pipeline layout */
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,
        .pushConstantRangeCount = 0
    };

    if (vkCreatePipelineLayout(context->device, &pipelineLayoutInfo, NULL, &pipeline->layout) != VK_SUCCESS) {
        MINIMAL_ERROR("Failed to create pipeline layout!");
        return MINIMAL_FAIL;
    }

    /* create pipeline */
    uint32_t vertexBindingDescCount = 0;
    VkVertexInputBindingDescription* vertexBindingDescs = getVertexBindingDescriptions(&vertexBindingDescCount);

    uint32_t vertexAttributeDescCount = 0;
    VkVertexInputAttributeDescription* vertexAttributeDescs = getVertexAttributeDescriptions(&vertexAttributeDescCount);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pVertexBindingDescriptions = vertexBindingDescs,
        .vertexBindingDescriptionCount = vertexBindingDescCount,
        .pVertexAttributeDescriptions = vertexAttributeDescs,
        .vertexAttributeDescriptionCount = vertexAttributeDescCount
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)context->swapchain.extent.width,
        .height = (float)context->swapchain.extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRect2D scissor = {
        .offset = { 0, 0 },
        .extent = context->swapchain.extent
    };

    VkPipelineViewportStateCreateInfo viewportState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor
    };

    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f, // Optional
        .depthBiasClamp = 0.0f, // Optional
        .depthBiasSlopeFactor = 0.0f // Optional
    };

    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT
                        | VK_COLOR_COMPONENT_G_BIT
                        | VK_COLOR_COMPONENT_B_BIT
                        | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_FALSE
    };

    VkPipelineColorBlendStateCreateInfo colorBlending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants[0] = 0.0f,
        .blendConstants[1] = 0.0f,
        .blendConstants[2] = 0.0f,
        .blendConstants[3] = 0.0f
    };

    VkGraphicsPipelineCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pStages = pipeline->shaderStages,
        .stageCount = SHADER_COUNT,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pColorBlendState = &colorBlending,
        .layout = pipeline->layout,
        .renderPass = context->swapchain.renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE
    };

    if (vkCreateGraphicsPipelines(context->device, VK_NULL_HANDLE, 1, &info, NULL, &pipeline->handle) != VK_SUCCESS) {
        return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
}

int pipelineRecreate(const VulkanContext* context, Pipeline* pipeline) {
    pipelineDestroy(context, pipeline);

    if (!pipelineCreate(context, pipeline)) {
        MINIMAL_ERROR("failed to recreate pipeline!");
        return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
}

void pipelineDestroy(const VulkanContext* context, Pipeline* pipeline) {
    vkDestroyPipeline(context->device, pipeline->handle, NULL);
    vkDestroyPipelineLayout(context->device, pipeline->layout, NULL);
}
