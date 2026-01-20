#pragma once

#include "render/AttachmentResources.h"
#include "render/RenderPass.h"
#include "render/VkContext.h"
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "render/RenderObject.h"

struct RenderGraphNodeDescription {
    uint32_t renderPass;
    std::vector<uint32_t> inputSamplers;
    uint32_t outputFramebuffer;
    std::vector<VkDescriptorSet> descriptors;
};

class RenderGraph {
public:
    RenderGraph(const VkContext* context);
    ~RenderGraph();

    const RenderPass* renderPass(uint32_t id) const;
    uint32_t addRenderPass(
        const VkFormat& format,
        const VkExtent2D& extent, 
        const std::vector<ShaderDescription>& shaders,
        const std::vector<VkDescriptorSetLayout> usedLayouts
    );
    uint32_t addNode(const RenderGraphNodeDescription& nodeDescription);
    void beginNode(uint32_t node, const VkCommandBuffer commandBuffer, const AttachmentResources* attachments) const;
    void endNode(uint32_t node, const VkCommandBuffer commandBuffer) const;

private:
    const VkContext* m_context;
    std::vector<RenderPass*> m_renderPasses;
    std::vector<RenderGraphNodeDescription> m_nodes;
};
