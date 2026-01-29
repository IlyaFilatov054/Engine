#pragma once

#include "render/DescriptorManager.h"
#include "render/Pipeline.h"
#include "render/VkContext.h"
#include <vector>
#include <vulkan/vulkan_core.h> 

struct RenderPassAttachmentDescription {
    VkFormat format;
    VkImageLayout initialLayout;
    VkImageLayout finalLayout;
    VkImageLayout referenceLayout;
    bool depth = false;
};

struct DescriptorSetLayoutReference {
    DescriptorSetLayoutHandle handle;
    VkDescriptorSetLayout layout;
};

struct RenderPassDescription {
    VkExtent2D extent;
    std::vector<RenderPassAttachmentDescription> attachments;
    std::vector<ShaderDescription> shaders;
    std::vector<DescriptorSetLayoutReference> descriptorSetLayouts;
};

class RenderPass {
public:
    RenderPass(
        const VkContext* context,
        const RenderPassDescription description
    );
    ~RenderPass();

    const VkRenderPass& renderPass() const;
    const Pipeline& pipeline() const;
    const VkExtent2D& extent() const;
    const std::vector<VkImageLayout>& outputLayouts() const;
    const std::vector<DescriptorSetLayoutHandle> descriptorOrder() const;
private:
    const VkContext* m_context;
    const VkExtent2D m_extent;

    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    Pipeline* m_pipeline = nullptr;
    std::vector<VkImageLayout> m_outputLayouts;
    std::vector<DescriptorSetLayoutHandle> m_descriptorOrder;
};