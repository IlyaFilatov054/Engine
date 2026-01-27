#pragma once

#include "render/AttachmentResources.h"
#include "render/RenderPass.h"
#include "render/StagedBuffer.h"
#include "render/VkContext.h"
#include <array>
#include <cstdint>
#include <functional>
#include <map>
#include <vector>
#include <vulkan/vulkan_core.h>

typedef std::function<void(
    const VkCommandBuffer commandBuffer,
    const RenderPass* renderPass,
    const AttachmentResources* attachments
)> DrawCall;

typedef uint32_t FrameRenderDataHandle;

enum DescriptorStage {
    Const = 0,
    Frame = 1,
    Input
};

typedef uint32_t RenderPassHandle;
struct RenderGraphNode {
    RenderPassHandle renderPass;
    std::vector<ReadAttachmentHandle> inputAttachments;
    WriteAttachmentHandle outputAttachment;
    std::vector<VkDescriptorSet> constDescriptors;
    std::vector<DescriptorAttachmentHandle> descriptorAttachments;
    std::array<DescriptorStage, 3> descriptorOrder = {Const, Frame, Input};
    std::vector<VkClearValue> clearValues;
    FrameRenderDataHandle frameRenderData;
};

typedef uint32_t NodeHandle;

struct ImageAttachmentDescription {
    std::vector<Image*> externalImages;
    VkFormat format;
    VkImageUsageFlags usage;
    VkExtent3D extent;
    VkImageAspectFlags aspect;
};

struct WriteAttachmentDescription {
    RenderPassHandle renderPass;
    VkExtent2D extent;
    std::vector<ImageAttachmentHandle> imageAttachments;
};

struct ReadAttachmentDescription {
    std::vector<VkDescriptorSet> perFrameDescriptors;
    std::vector<ImageAttachmentHandle> imageAttachments;
};

typedef std::function<void*()> DescriptorUpdateSource;
struct DescriptorAttachmentDescription {
    std::vector<VkDescriptorSet> perFrameDescriptors;
    VkDescriptorType type;
    VkBufferUsageFlagBits usage;
    uint32_t bufferSize;
    DescriptorUpdateSource updateSource;
};

typedef uint32_t StepHandle;

class RenderGraph {
public:
    RenderGraph(const VkContext* context);
    ~RenderGraph();

    const RenderPassHandle addRenderPass(
        const std::vector<VkAttachmentDescription>& attachments,
        const std::vector<VkImageLayout>& attachmentLayouts,
        const VkExtent2D& extent, 
        const std::vector<ShaderDescription>& shaders,
        const std::vector<VkDescriptorSetLayout> usedLayouts
    );
    const NodeHandle addNode(const RenderGraphNode& node, StepHandle step);
    void execute(const VkCommandBuffer commandBuffer, uint32_t image);
    void addDrawCall(FrameRenderDataHandle frameRenderDataHandle, DrawCall drawCall);
    
    const ImageAttachmentHandle addImageAttachment(ImageAttachmentDescription& description);
    const WriteAttachmentHandle addWriteAttachment(WriteAttachmentDescription& description);
    const ReadAttachmentHandle addReadAttachment(ReadAttachmentDescription& description);
    const DescriptorAttachmentHandle addDescriptorAttachment(DescriptorAttachmentDescription& description);
    void createAttachmentResources(uint32_t imageCount);
private:
    const VkContext* m_context;
    
    std::vector<RenderPass*> m_renderPasses;
    
    std::vector<RenderGraphNode> m_nodes;
    std::map<StepHandle, std::vector<NodeHandle>> m_steps;
    std::map<FrameRenderDataHandle, std::vector<DrawCall>> m_renderQueue;

    std::vector<AttachmentResources*> m_attachmentResources;
    std::vector<ImageAttachmentDescription> m_imageAttachmentDescriptions;
    std::vector<WriteAttachmentDescription> m_writeAttachmentDescriptions;
    std::vector<ReadAttachmentDescription> m_readAttachmentDescriptions;
    std::vector<DescriptorAttachmentDescription> m_descriptorAttachmentDescriptions;

    void executeNode(
        NodeHandle node,
        const VkCommandBuffer commandBuffer,
        const AttachmentResources* attachments
    );
};
