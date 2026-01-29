#include "render/RenderPass.h"
#include "render/Pipeline.h"
#include "render/VkUtils.h"
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>

RenderPass::RenderPass(
    const VkContext* context,
    const std::vector<VkAttachmentDescription>& attachments,
    const std::vector<VkImageLayout>& attachmentLayouts,
    const VkExtent2D& extent,
    const std::vector<ShaderDescription>& shaders,
    const std::vector<std::pair<DescriptorSetLayoutHandle, VkDescriptorSetLayout>> usedLayouts) 
: m_context(context), m_extent(extent) {
    std::vector<VkAttachmentReference> colorReferences;
    VkAttachmentReference depthReference;
    int depthAttachment = -1;
    for(uint32_t i = 0; i < attachments.size(); i++) {
        m_outputLayouts.push_back(attachments[i].finalLayout);
        if(attachments[i].format == VK_FORMAT_D32_SFLOAT) {
            depthAttachment = static_cast<uint32_t>(i);
            depthReference = {
                .attachment = i,
                .layout = attachmentLayouts[i]
            };
            continue;
        }
        colorReferences.push_back({
            .attachment = i,
            .layout = attachmentLayouts[i]
        });
    }

    VkSubpassDescription subpassDescription {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = static_cast<uint32_t>(colorReferences.size()),
        .pColorAttachments = colorReferences.data(),
        .pDepthStencilAttachment = depthAttachment > -1 ? &depthReference : nullptr
    };

    VkSubpassDependency subpassDependency {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    };

    VkRenderPassCreateInfo renderPassCreateInfo {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpassDescription,
        .dependencyCount = 1,
        .pDependencies = &subpassDependency
    };

    auto res = vkCreateRenderPass(m_context->device(), &renderPassCreateInfo, nullptr, &m_renderPass);
    validateVkResult(res, "vkCreateRenderPass");

    std::vector<VkDescriptorSetLayout> layouts;
    for(auto l : usedLayouts) {
        m_descriptorOrder.push_back(l.first);
        layouts.push_back(l.second);
    }

    m_pipeline = new Pipeline(m_context, extent, m_renderPass, shaders, layouts);
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

const std::vector<VkImageLayout>& RenderPass::outputLayouts() const {
    return m_outputLayouts;
}

const std::vector<DescriptorSetLayoutHandle> RenderPass::descriptorOrder() const {
    return m_descriptorOrder;
}