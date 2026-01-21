#pragma once

#include "render/AttachmentResources.h"
#include "render/MeshBuffer.h"
#include "render/RenderPass.h"
#include "render/VkContext.h"
#include <cstdint>
#include <map>
#include <vector>
#include <vulkan/vulkan_core.h>

struct DrawCall {
    uint32_t pushConstant;
    MeshBuffer* mesh;
};

struct RenderGraphNode {
    uint32_t renderPass;
    std::vector<uint32_t> inputSamplers;
    uint32_t outputFramebuffer;
    std::vector<VkDescriptorSet> constDescriptors;
    std::vector<uint32_t> frameDescriptors;
    std::vector<DrawCall> drawCalls;
    bool clearDrawCalls;
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
    uint32_t addNode(const RenderGraphNode& node, uint32_t step);
    void execute(const VkCommandBuffer commandBuffer, const AttachmentResources* attachments);
    void addDrawCall(uint32_t node, const DrawCall drawCall);

private:
    const VkContext* m_context;
    std::vector<RenderPass*> m_renderPasses;
    std::vector<RenderGraphNode> m_nodes;
    std::map<uint32_t, std::vector<uint32_t>> m_steps;

    void executeNode(uint32_t node, const VkCommandBuffer commandBuffer, const AttachmentResources* attachments);
};
