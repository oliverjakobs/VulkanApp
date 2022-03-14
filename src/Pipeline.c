#include "Pipeline.h"

#include "Minimal/Utils.h"

VkShaderModule createShaderModule(VulkanContext* context, const char* path) {
    size_t size = 0;
    char* code = readSPIRV(path, &size);

    if (!code) return VK_NULL_HANDLE;

    VkShaderModuleCreateInfo info = { 
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = size,
        .pCode = (const uint32_t*)code
    };

    VkShaderModule shaderModule;
    VkResult result = vkCreateShaderModule(context->device, &info, NULL, &shaderModule);

    free(code);

    if (result != VK_SUCCESS) {
        MINIMAL_ERROR("failed to create shader module for %s", path);
        return VK_NULL_HANDLE;
    }

    return shaderModule;
}

int createGraphicsPipeline(VulkanContext* context) {
    VkShaderModule vert = createShaderModule(context, "res/shader/vert.spv");
    VkShaderModule frag = createShaderModule(context, "res/shader/frag.spv");

    if (vert == VK_NULL_HANDLE || frag == VK_NULL_HANDLE) {
        vkDestroyShaderModule(context->device, frag, NULL);
        vkDestroyShaderModule(context->device, vert, NULL);
        return MINIMAL_FAIL;
    }

    VkPipelineShaderStageCreateInfo shaderStages[2] = { 0 };

    /* vertex shader */
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = vert;
    shaderStages[0].pName = "main";

    /* fragment shader */
    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = frag;
    shaderStages[1].pName = "main";

    vkDestroyShaderModule(context->device, vert, NULL);
    vkDestroyShaderModule(context->device, frag, NULL);

    return MINIMAL_OK;
}