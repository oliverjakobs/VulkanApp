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

    /* descriptor layout */
    VkDescriptorSetLayoutBinding descriptorBindings[] = {
        {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = NULL
        },
        {
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = NULL
        }
    };

    uint32_t bindingCount = sizeof(descriptorBindings) / sizeof(VkDescriptorSetLayoutBinding);

    VkDescriptorSetLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = bindingCount,
        .pBindings = descriptorBindings
    };

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, allocator, &pipeline->descriptorSetLayout) != VK_SUCCESS)
        return IGNIS_FAIL;

    /* descriptor pool */
    VkDescriptorPoolSize poolSizes[] = {
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = IGNIS_MAX_FRAMES_IN_FLIGHT
        },
        {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = IGNIS_MAX_FRAMES_IN_FLIGHT
        }
    };

    uint32_t poolSizeCount = sizeof(poolSizes) / sizeof(VkDescriptorPoolSize);

    VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = poolSizeCount,
        .pPoolSizes = poolSizes,
        .maxSets = IGNIS_MAX_FRAMES_IN_FLIGHT
    };

    /* uniform buffer */
    pipeline->uniformBufferSize = sizeof(UniformBufferObject);

    for (size_t i = 0; i < IGNIS_MAX_FRAMES_IN_FLIGHT; ++i)
    {
        ignisCreateBuffer(NULL, pipeline->uniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &pipeline->uniformBuffers[i]);
        vkMapMemory(device, pipeline->uniformBuffers[i].memory, 0, pipeline->uniformBufferSize, 0, &pipeline->uniformBufferData[i]);
    }

    if (vkCreateDescriptorPool(device, &poolInfo, allocator, &pipeline->descriptorPool) != VK_SUCCESS)
    {
        IGNIS_ERROR("failed to create descriptor pool!");
        return IGNIS_FAIL;
    }

    /* descriptor sets */
    for (size_t i = 0; i < IGNIS_MAX_FRAMES_IN_FLIGHT; ++i)
    {
        VkDescriptorSetAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = pipeline->descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &pipeline->descriptorSetLayout
        };

        if (vkAllocateDescriptorSets(device, &allocInfo, &pipeline->descriptorSets[i]) != VK_SUCCESS) {
            IGNIS_ERROR("failed to allocate descriptor sets!");
        }

        VkDescriptorBufferInfo bufferInfo = {
            .buffer = pipeline->uniformBuffers[i].handle,
            .offset = 0,
            .range = VK_WHOLE_SIZE
        };

        VkWriteDescriptorSet descriptorWrite = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = pipeline->descriptorSets[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .pBufferInfo = &bufferInfo
        };

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, NULL);
    }

    /* layout */
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &pipeline->descriptorSetLayout,
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
        .cullMode = config->cullMode,
        .frontFace = config->frontFace,
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

    for (size_t i = 0; i < IGNIS_MAX_FRAMES_IN_FLIGHT; ++i)
        ignisDestroyBuffer(&pipeline->uniformBuffers[i]);

    vkDestroyPipeline(device, pipeline->handle, allocator);
    vkDestroyPipelineLayout(device, pipeline->layout, allocator);

    vkDestroyDescriptorPool(device, pipeline->descriptorPool, allocator);
    vkDestroyDescriptorSetLayout(device, pipeline->descriptorSetLayout, allocator);
}

void ignisBindPipeline(VkCommandBuffer commandBuffer, IgnisPipeline* pipeline)
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle);

    uint32_t frame = ignisGetCurrentFrame();
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout, 0, 1, &pipeline->descriptorSets[frame], 0, NULL);
}

uint8_t ignisPushUniform(IgnisPipeline* pipeline, const void* data, uint32_t size, uint32_t offset)
{
    if (offset + size > pipeline->uniformBufferSize)
        return IGNIS_FAIL;

    uint32_t frame = ignisGetCurrentFrame();

    memcpy((char*)pipeline->uniformBufferData[frame] + offset, data, size);

    return IGNIS_OK;
}

uint8_t ignisBindTexture(IgnisPipeline* pipeline, const IgnisTexture* texture, uint32_t binding)
{
    VkDevice device = ignisGetVkDevice();
    uint32_t frame = ignisGetCurrentFrame();

    VkDescriptorImageInfo imageInfo = {
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .imageView = texture->view,
        .sampler = texture->sampler,
    };

    VkWriteDescriptorSet descriptorWrite = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = pipeline->descriptorSets[frame],
        .dstBinding = binding,
        .dstArrayElement = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .pImageInfo = &imageInfo
    };

    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, NULL);

    return IGNIS_OK;
}
