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

const RenderPass* RenderGraph::renderPass(uint32_t id) const {
    return m_renderPasses[id];
}

uint32_t RenderGraph::addRenderPass(
    const std::vector<VkAttachmentDescription>& attachments,
    const std::vector<VkImageLayout>& attachmentLayouts,
    const VkExtent2D& extent, 
    const std::vector<ShaderDescription>& shaders,
    const std::vector<VkDescriptorSetLayout> usedLayouts
) {
    m_renderPasses.push_back(new RenderPass(m_context, attachments, attachmentLayouts, extent, shaders, usedLayouts));
    return m_renderPasses.size() - 1;
}

uint32_t RenderGraph::addNode(const RenderGraphNode& node, uint32_t step) {
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

void RenderGraph::addDrawCall(uint32_t node, const DrawCall drawCall) {
    m_nodes[node].drawCalls.push_back(drawCall);
}

void RenderGraph::executeNode(uint32_t node, const VkCommandBuffer commandBuffer, const AttachmentResources* attachments) {
    auto& n = m_nodes[node];
    auto pass = renderPass(n.renderPass);
    
    VkRenderPassBeginInfo renderPassInfo {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = pass->renderPass(),
        .framebuffer = attachments->writeAttachment(n.outputFramebuffer).framebuffer,
        .renderArea = {.offset = {0, 0}, .extent = renderPass(n.renderPass)->extent()},
        .clearValueCount = static_cast<uint32_t>(n.clearValues.size()),
        .pClearValues = n.clearValues.data(),
    };
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pass->pipeline().pipeline());

    std::vector<VkDescriptorSet> descriptors;
    descriptors.insert(descriptors.end(), n.constDescriptors.begin(), n.constDescriptors.end());
    for(uint32_t i = 0; i < n.frameDescriptors.size(); i++) {
        descriptors.push_back(attachments->descriptorAttachment(i).descriptor);
    }
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pass->pipeline().layout(), 0, descriptors.size(), descriptors.data(), 0, nullptr);

    if(n.drawCalls.size() > 0) {
        for(const auto& d : n.drawCalls) {
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
    if(n.clearDrawCalls) n.drawCalls.clear();

    vkCmdEndRenderPass(commandBuffer);
}