#include "render/RenderGraph.h"
#include "render/RenderPass.h"
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>

RenderGraph::RenderGraph(const VkContext* context) : m_context(context) {

}

RenderGraph::~RenderGraph() {
    for(const auto& p : m_renderPasses){
        delete p;
    }    
}

const RenderPass* RenderGraph::renderPass(RenderPassHandle handle) const {
    return m_renderPasses[handle];
}

const RenderPassHandle RenderGraph::addRenderPass(
    const std::vector<VkAttachmentDescription>& attachments,
    const std::vector<VkImageLayout>& attachmentLayouts,
    const VkExtent2D& extent, 
    const std::vector<ShaderDescription>& shaders,
    const std::vector<VkDescriptorSetLayout> usedLayouts
) {
    m_renderPasses.push_back(new RenderPass(m_context, attachments, attachmentLayouts, extent, shaders, usedLayouts));
    return m_renderPasses.size() - 1;
}

const NodeHandle RenderGraph::addNode(const RenderGraphNode& node, uint32_t step) {
    m_nodes.push_back(node);
    uint32_t id = m_nodes.size() - 1;

    auto it = m_steps.find(step);
    if(it == m_steps.end()) m_steps[step] = std::vector<uint32_t>();
    m_steps[step].push_back(id);

    return id;
}

void RenderGraph::execute(const VkCommandBuffer commandBuffer, const AttachmentResources* attachments) {
    for (auto it = m_steps.begin(); it != m_steps.end(); it++) {
        for(auto& n : it->second){
            executeNode(n, commandBuffer, attachments);
        }
    }
}

void RenderGraph::addDrawCall(NodeHandle nodeHandle, DrawCall drawCall) {
    m_nodes[nodeHandle].drawCalls.push_back(drawCall);
}

void RenderGraph::executeNode(NodeHandle nodeHandle, const VkCommandBuffer commandBuffer, const AttachmentResources* attachments) {
    auto& node = m_nodes[nodeHandle];
    auto pass = renderPass(node.renderPass);

    for(auto inputAttachmentHandle : node.inputAttachments) {
        auto inputAttachment = attachments->readAttachment(inputAttachmentHandle);
        for (uint32_t i = 0; i < inputAttachment.images.size(); i++) {
            auto inputAttachmentImage = attachments->imageAttachment(inputAttachment.images[i]);
            inputAttachmentImage.image->setLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            inputAttachmentImage.image->setPipelineStage(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
            inputAttachmentImage.image->setAccessMask(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
            inputAttachmentImage.image->transitionLayout(
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                VK_ACCESS_SHADER_READ_BIT,
                commandBuffer
            );
        }
    }

    VkRenderPassBeginInfo renderPassInfo {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = pass->renderPass(),
        .framebuffer = attachments->writeAttachment(node.outputAttachment).framebuffer,
        .renderArea = {.offset = {0, 0}, .extent = renderPass(node.renderPass)->extent()},
        .clearValueCount = static_cast<uint32_t>(node.clearValues.size()),
        .pClearValues = node.clearValues.data(),
    };
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pass->pipeline().pipeline());

    std::vector<VkDescriptorSet> descriptors;
    descriptors.insert(descriptors.end(), node.constDescriptors.begin(), node.constDescriptors.end());
    for(uint32_t i = 0; i < node.descriptorAttachments.size(); i++) {
        descriptors.push_back(attachments->descriptorAttachment(node.descriptorAttachments[i]).descriptor);
    }
    for(auto inputAttachmentHandle : node.inputAttachments) {
        descriptors.push_back(attachments->readAttachment(inputAttachmentHandle).descriptor);
    }
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pass->pipeline().layout(), 0, descriptors.size(), descriptors.data(), 0, nullptr);

    if(node.drawCalls.size() > 0) {
        for(const auto& d : node.drawCalls) {
            vkCmdPushConstants(commandBuffer, pass->pipeline().layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uint32_t), &d.pushConstant);
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &d.mesh->vertexBuffer(), offsets);
            vkCmdBindIndexBuffer(commandBuffer, d.mesh->indexBuffer(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(commandBuffer, d.mesh->indicesCount(), 1, 0, 0, 0);
        }
    }
    else {
        vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    }
    if(node.clearDrawCalls) node.drawCalls.clear();

    vkCmdEndRenderPass(commandBuffer);
}