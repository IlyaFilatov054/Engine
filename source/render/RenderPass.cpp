#include "render/RenderPass.h"
#include "render/Pipeline.h"
#include "render/VkUtils.h"

RenderPass::RenderPass(const VkContext* context, const VkFormat& format, const VkExtent2D& extent,
    const std::vector<ShaderDescription>& shaders, const std::vector<VkDescriptorSetLayout> usedLayouts) 
: m_context(context), m_format(format), m_extent(extent) {
    VkAttachmentDescription colorAttachment {
        .format = m_format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };
    VkAttachmentReference colorAttachmentReference {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkAttachmentDescription depthAttachment {
        .format = VK_FORMAT_D32_SFLOAT,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };
    VkAttachmentReference depthAttachmentReference {
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpassDescription {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentReference,
        .pDepthStencilAttachment = &depthAttachmentReference
    };

    VkSubpassDependency subpassDependency {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    };

    VkAttachmentDescription attachments[] = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassCreateInfo {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 2,
        .pAttachments = attachments,
        .subpassCount = 1,
        .pSubpasses = &subpassDescription,
        .dependencyCount = 1,
        .pDependencies = &subpassDependency
    };

    auto res = vkCreateRenderPass(m_context->device(), &renderPassCreateInfo, nullptr, &m_renderPass);
    validateVkResult(res, "vkCreateRenderPass");

    m_pipeline = new Pipeline(m_context, extent, m_renderPass, shaders, usedLayouts);
};

RenderPass::~RenderPass() {
    delete m_pipeline;
    vkDestroyRenderPass(m_context->device(), m_renderPass, nullptr);
}

const VkRenderPass& RenderPass::renderPass() const {
    return m_renderPass;
}

const Pipeline& RenderPass::pipeline() const {
    return *m_pipeline;
}

const VkExtent2D& RenderPass::extent() const {
    return m_extent;
}