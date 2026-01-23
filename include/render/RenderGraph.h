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

typedef uint32_t RenderPassHandle;
struct RenderGraphNode {
    RenderPassHandle renderPass;
    std::vector<ReadAttachmentHandle> inputAttachments;
    WriteAttachmentHandle outputAttachment;
    std::vector<VkDescriptorSet> constDescriptors;
    std::vector<DescriptorAttachmentHandle> descriptorAttachments;
    std::vector<DrawCall> drawCalls;
    bool clearDrawCalls;
    std::vector<VkClearValue> clearValues;
};

typedef uint32_t NodeHandle;

class RenderGraph {
public:
    RenderGraph(const VkContext* context);
    ~RenderGraph();

    const RenderPass* renderPass(RenderPassHandle handle) const;
    const RenderPassHandle addRenderPass(
        const std::vector<VkAttachmentDescription>& attachments,
        const std::vector<VkImageLayout>& attachmentLayouts,
        const VkExtent2D& extent, 
        const std::vector<ShaderDescription>& shaders,
        const std::vector<VkDescriptorSetLayout> usedLayouts
    );
    const NodeHandle addNode(const RenderGraphNode& node, uint32_t step);
    void execute(const VkCommandBuffer commandBuffer, const AttachmentResources* attachments);
    void addDrawCall(NodeHandle nodeHandle, const DrawCall drawCall);

private:
    const VkContext* m_context;
    std::vector<RenderPass*> m_renderPasses;
    std::vector<RenderGraphNode> m_nodes;
    std::map<uint32_t, std::vector<uint32_t>> m_steps;

    void executeNode(NodeHandle node, const VkCommandBuffer commandBuffer, const AttachmentResources* attachments);
};
