#pragma once

#include "render/AttachmentResources.h"
#include "render/DescriptorManager.h"
#include "render/RenderPass.h"
#include "render/VkContext.h"
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

typedef uint32_t RenderPassHandle;

struct RenderGraphNode {
    RenderPassHandle renderPass;
    std::vector<ReadAttachmentHandle> inputAttachments;
    WriteAttachmentHandle outputAttachment;
    std::vector<DescriptorSet> externalDescriptors;
    std::vector<DescriptorAttachmentHandle> descriptorAttachments;
    std::vector<VkClearValue> clearValues;
    FrameRenderDataHandle frameRenderData;
};

typedef uint32_t NodeHandle;

struct ImageAttachmentDescription {
    ImageAttachmentHandle handle;
    std::vector<Image*> externalImages;
    VkFormat format;
    VkImageUsageFlags usage;
    VkExtent3D extent;
    VkImageAspectFlags aspect;
};

struct WriteAttachmentDescription {
    WriteAttachmentHandle handle;
    RenderPassHandle renderPass;
    VkExtent2D extent;
    std::vector<ImageAttachmentHandle> imageAttachments;
};

struct ReadAttachmentDescription {
    ReadAttachmentHandle handle;
    std::vector<DescriptorSet> perFrameDescriptors;
    std::vector<ImageAttachmentHandle> imageAttachments;
};

typedef std::function<void*()> DescriptorUpdateSource;
struct DescriptorAttachmentDescription {
    DescriptorAttachmentHandle handle;
    std::vector<DescriptorSet> perFrameDescriptors;
    VkDescriptorType type;
    VkBufferUsageFlagBits usage;
    uint32_t bufferSize;
};

typedef uint32_t StepHandle;

class RenderGraph {
public:
    RenderGraph(const VkContext* context);
    ~RenderGraph();

    void addRenderPass(
        RenderPassHandle handle,
        const std::vector<VkAttachmentDescription>& attachments,
        const std::vector<VkImageLayout>& attachmentLayouts,
        const VkExtent2D& extent, 
        const std::vector<ShaderDescription>& shaders,
        const std::vector<DescriptorSetLayout> usedLayouts
    );
    void addNode(NodeHandle handle, const RenderGraphNode& node, StepHandle step);
    void execute(const VkCommandBuffer commandBuffer, uint32_t image);
    void addDrawCall(FrameRenderDataHandle frameRenderDataHandle, DrawCall drawCall);
    
    void addImageAttachment(ImageAttachmentDescription& description);
    void addWriteAttachment(WriteAttachmentDescription& description);
    void addReadAttachment(ReadAttachmentDescription& description);
    void addDescriptorAttachment(DescriptorAttachmentDescription& description);
    void bindDescriptorAttachmentDataSource(DescriptorAttachmentHandle handle, DescriptorUpdateSource source);
    void createAttachmentResources(uint32_t imageCount);
private:
    const VkContext* m_context;
    
    std::map<RenderPassHandle, RenderPass*> m_renderPasses;
    
    std::map<NodeHandle, RenderGraphNode> m_nodes;
    std::map<StepHandle, std::vector<NodeHandle>> m_steps;
    std::map<FrameRenderDataHandle, std::vector<DrawCall>> m_renderQueue;

    std::vector<AttachmentResources*> m_attachmentResources;
    std::vector<ImageAttachmentDescription> m_imageAttachmentDescriptions;
    std::vector<WriteAttachmentDescription> m_writeAttachmentDescriptions;
    std::vector<ReadAttachmentDescription> m_readAttachmentDescriptions;
    std::vector<DescriptorAttachmentDescription> m_descriptorAttachmentDescriptions;
    std::map<DescriptorAttachmentHandle, DescriptorUpdateSource> m_descriptorAttachmentsUpdateSources;

    void executeNode(
        NodeHandle node,
        const VkCommandBuffer commandBuffer,
        const AttachmentResources* attachments
    );
};
