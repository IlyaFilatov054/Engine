#include "render/RenderGraph.h"
#include "render/RenderPass.h"
#include <cstdint>

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
    const VkFormat& format,
    const VkExtent2D& extent, 
    const std::vector<ShaderDescription>& shaders,
    const std::vector<VkDescriptorSetLayout> usedLayouts
) {
    m_renderPasses.push_back(new RenderPass(m_context, format, extent, shaders, usedLayouts));
    return m_renderPasses.size() - 1;
}

uint32_t RenderGraph::addNode(const RenderGraphNodeDescription& nodeDescription) {
    m_nodes.push_back(nodeDescription);
    return m_nodes.size() - 1;
}

void RenderGraph::beginNode(uint32_t node, const VkCommandBuffer commandBuffer, const AttachmentResources* attachments) const {
    const auto& n = m_nodes[node];
    VkClearValue clearValues[2];
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    VkRenderPassBeginInfo renderPassInfo {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass(n.renderPass)->renderPass(),
        .framebuffer = attachments->writeAttachment(n.outputFramebuffer).framebuffer,
        .renderArea = {.offset = {0, 0}, .extent = renderPass(n.renderPass)->extent()},
        .clearValueCount = 2,
        .pClearValues = clearValues
    };
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass(n.renderPass)->pipeline().pipeline());
}

void RenderGraph::endNode(uint32_t node, const VkCommandBuffer commandBuffer) const {
    vkCmdEndRenderPass(commandBuffer);
}