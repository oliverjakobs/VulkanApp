#include "pipeline.h"

#include "ignis.h"

static VkShaderModule ignisCreateShaderModule(VkDevice device, const char* path, const VkAllocationCallbacks* allocator)
{
    size_t size;
    char* code = ignisReadFile(path, &size);

    VkShaderModuleCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = size - 1,
        .pCode = (const uint32_t*)code
    };

    VkShaderModule module;
    VkResult result = vkCreateShaderModule(device, &info, allocator, &module);

    ignisFree(code, size);

    if (result != VK_SUCCESS) return VK_NULL_HANDLE;

    return module;
}

uint8_t ignisCreatePipeline(const IgnisPipelineConfig* config, IgnisPipeline* pipeline)
{
    VkDevice device = ignisGetVkDevice();
    const VkAllocationCallbacks* allocator = ignisGetAllocator();

    /* layout */
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,
        .pushConstantRangeCount = 0,
    };

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, allocator, &pipeline->layout) != VK_SUCCESS)
        return IGNIS_FAIL;

    /* shader */
    VkShaderModule vertModule = ignisCreateShaderModule(device, config->vertPath, allocator);
    VkShaderModule fragModule = ignisCreateShaderModule(device, config->fragPath, allocator);

    VkPipelineShaderStageCreateInfo shaderStages[] = {
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertModule,
            .pName = "main",
        },
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragModule,
            .pName = "main",
        }
    };

    /* vertex input */
    VkVertexInputBindingDescription bindingDescription = {
        .binding = 0,
        .stride = config->vertexStride,
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = config->attributeCount,
        .pVertexAttributeDescriptions = config->vertexAttributes
    };

    /* input assembly */
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    /* viewport */
    VkPipelineViewportStateCreateInfo viewportState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1
    };

    /* rasterization */
    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE
    };

    /* multisample */
    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
    };

    /* depth stencil */
    VkPipelineDepthStencilStateCreateInfo depthStencil = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE
    };

    /* color blending */
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

    /* dynamic states */
    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicStateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = sizeof(dynamicStates) / sizeof(dynamicStates[0]),
        .pDynamicStates = dynamicStates
    };

    /* create pipeline */
    VkGraphicsPipelineCreateInfo pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = &depthStencil,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicStateInfo,
        .layout = pipeline->layout,
        .renderPass = ignisGetVkRenderPass(),
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE
    };

    VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, allocator, &pipeline->handle);

    vkDestroyShaderModule(device, vertModule, allocator);
    vkDestroyShaderModule(device, fragModule, allocator);

    return result != VK_SUCCESS ? IGNIS_FAIL : IGNIS_OK;
}

void ignisDestroyPipeline(IgnisPipeline* pipeline)
{
    VkDevice device = ignisGetVkDevice();
    const VkAllocationCallbacks* allocator = ignisGetAllocator();

    vkDestroyPipeline(device, pipeline->handle, allocator);
    vkDestroyPipelineLayout(device, pipeline->layout, allocator);
}