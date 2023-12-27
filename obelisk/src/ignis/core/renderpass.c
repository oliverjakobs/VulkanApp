#include "../ignis_core.h"

uint8_t ignisCreateRenderPass(VkDevice device, VkFormat imageFormat, VkFormat depthFormat, IgnisRenderPass* renderPass)
{
    VkAttachmentDescription attachments[] = { 
        {
            // colorAttachment
            .format = imageFormat,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        },
        {
            // depthAttachment
            .format = depthFormat,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        }
    };

    VkAttachmentReference attachmentRefs[] = {
        { .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
        { .attachment = 1, .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL }
    };

    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .pColorAttachments = attachmentRefs,
        .colorAttachmentCount = 1,
        .pDepthStencilAttachment = &attachmentRefs[1]
    };

    VkSubpassDependency dependency = {
        .dstSubpass = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .srcAccessMask = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };

    VkRenderPassCreateInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pAttachments = attachments,
        .attachmentCount = sizeof(attachments) / sizeof(VkAttachmentDescription),
        .pSubpasses = &subpass,
        .subpassCount = 1,
        .pDependencies = &dependency,
        .dependencyCount = 1
    };

    if (vkCreateRenderPass(device, &renderPassInfo, ignisGetAllocator(), &renderPass->handle) != VK_SUCCESS)
    {
        IGNIS_ERROR("failed to create render pass!");
        return IGNIS_FAIL;
    }

    return IGNIS_OK;
}

void ignisDestroyRenderPass(VkDevice device, IgnisRenderPass* renderPass)
{
    vkDestroyRenderPass(device, renderPass->handle, ignisGetAllocator());
}