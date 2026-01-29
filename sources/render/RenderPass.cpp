#include "render/RenderPass.h"
#include "render/Pipeline.h"
#include "render/VkUtils.h"
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>

RenderPass::RenderPass(
    const VkContext* context,
    const RenderPassDescription description
) 
: m_context(context), m_extent(description.extent) {
    std::vector<VkAttachmentReference> colorReferences;
    VkAttachmentReference depthReference;
    int depthAttachment = -1;
    for(uint32_t i = 0; i < description.attachments.size(); i++) {
        m_outputLayouts.push_back(description.attachments[i].finalLayout);
        if(description.attachments[i].depth) {
            depthAttachment = static_cast<uint32_t>(i);
            depthReference = {
                .attachment = i,
                .layout = description.attachments[i].referenceLayout
            };
            continue;
        }
        colorReferences.push_back({
            .attachment = i,
            .layout = description.attachments[i].referenceLayout
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

    std::vector<VkAttachmentDescription> attachmentDescriptions;
    for(auto attachment : description.attachments) {
        VkAttachmentDescription attachmentDescription {
            .format = attachment.format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = attachment.initialLayout,
            .finalLayout = attachment.finalLayout
        };
        attachmentDescriptions.push_back(attachmentDescription);
    }

    VkRenderPassCreateInfo renderPassCreateInfo {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size()),
        .pAttachments = attachmentDescriptions.data(),
        .subpassCount = 1,
        .pSubpasses = &subpassDescription,
        .dependencyCount = 1,
        .pDependencies = &subpassDependency
    };

    auto res = vkCreateRenderPass(m_context->device(), &renderPassCreateInfo, nullptr, &m_renderPass);
    validateVkResult(res, "vkCreateRenderPass");

    std::vector<VkDescriptorSetLayout> layouts;
    for(auto l : description.descriptorSetLayouts) {
        m_descriptorOrder.push_back(l.handle);
        layouts.push_back(l.layout);
    }

    m_pipeline = new Pipeline(m_context, description.extent, m_renderPass, description.shaders, layouts);
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