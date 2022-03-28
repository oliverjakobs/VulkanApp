#include "Pipeline.h"

#include "Buffer.h"

static int createShaderModuleSrc(const VulkanContext* context, VkShaderModule* module, const uint32_t* code, size_t size) {
    VkShaderModuleCreateInfo info = { 
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = size,
        .pCode = (const uint32_t*)code
    };

    if (vkCreateShaderModule(context->device, &info, NULL, module) != VK_SUCCESS) {
        return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
}

static int createShaderModuleSPIRV(const VulkanContext* context, VkShaderModule* module, const char* path) {
    size_t size = 0;
    char* code = readFile(path, &size);
    if (!code) return MINIMAL_FAIL;

    int result = createShaderModuleSrc(context, module, (const uint32_t*)code, size);
    free(code);
    return result;
}

int createShaderStages(const VulkanContext* context, Pipeline* pipeline, const char* vertPath, const char* fragPath) {
    if (!createShaderModuleSPIRV(context, &pipeline->shaderModules[SHADER_VERT], vertPath)) {
        MINIMAL_ERROR("failed to create shader module for %s", vertPath);
        return MINIMAL_FAIL;
    }

    if (!createShaderModuleSPIRV(context, &pipeline->shaderModules[SHADER_FRAG], fragPath)) {
        MINIMAL_ERROR("failed to create shader module for %s", fragPath);
        return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
}

void destroyShaderStages(const VulkanContext* context, Pipeline* pipeline) {
    for (size_t i = 0; i < SHADER_COUNT; ++i) {
        vkDestroyShaderModule(context->device, pipeline->shaderModules[i], NULL);
    }
}

int createPipelineLayout(const VulkanContext* context, Pipeline* pipeline, const VertexInputDescription* desc) {

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pSetLayouts = &context->descriptorSetLayout,
        .setLayoutCount = 1,
        .pPushConstantRanges = NULL,
        .pushConstantRangeCount = 0
    };

    if (vkCreatePipelineLayout(context->device, &pipelineLayoutInfo, NULL, &pipeline->layout) != VK_SUCCESS) {
        return MINIMAL_FAIL;
    }

    pipeline->inputDesc = *desc;

    return MINIMAL_OK;
}

void destroyPipelineLayout(const VulkanContext* context, Pipeline* pipeline) {
    vkDestroyPipelineLayout(context->device, pipeline->layout, NULL);
}

int createPipeline(const VulkanContext* context, Pipeline* pipeline) {
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
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pVertexBindingDescriptions = pipeline->inputDesc.bindings,
        .vertexBindingDescriptionCount = pipeline->inputDesc.bindingCount,
        .pVertexAttributeDescriptions = pipeline->inputDesc.attributes,
        .vertexAttributeDescriptionCount = pipeline->inputDesc.attributeCount
    };

    /* input assembly state */
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    /* viewport state */
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
        .pViewports = &viewport,
        .viewportCount = 1,
        .pScissors = &scissor,
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
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants[0] = 0.0f,
        .blendConstants[1] = 0.0f,
        .blendConstants[2] = 0.0f,
        .blendConstants[3] = 0.0f
    };

    /* dynamic state */

    /* create pipeline */
    VkGraphicsPipelineCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pStages = shaderStages,
        .stageCount = SHADER_COUNT,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = NULL,
        .pColorBlendState = &colorBlending,
        .pDynamicState = NULL,
        .layout = pipeline->layout,
        .renderPass = context->swapchain.renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1
    };

    if (vkCreateGraphicsPipelines(context->device, VK_NULL_HANDLE, 1, &info, NULL, &pipeline->handle) != VK_SUCCESS) {
        return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
}

int recreatePipeline(const VulkanContext* context, Pipeline* pipeline) {
    destroyPipeline(context, pipeline);

    if (!createPipeline(context, pipeline)) {
        MINIMAL_ERROR("failed to recreate pipeline!");
        return MINIMAL_FAIL;
    }

    return MINIMAL_OK;
}

void destroyPipeline(const VulkanContext* context, Pipeline* pipeline) {
    vkDestroyPipeline(context->device, pipeline->handle, NULL);
}
