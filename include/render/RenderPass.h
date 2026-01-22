#pragma once

#include "render/Pipeline.h"
#include "render/VkContext.h"
#include <vector>
#include <vulkan/vulkan_core.h> 

class RenderPass {
public:
    RenderPass(
        const VkContext* context,
        const std::vector<VkAttachmentDescription>& attachments,
        const std::vector<VkImageLayout>& attachmentLayouts,
        const VkExtent2D& extent, 
        const std::vector<ShaderDescription>& shaders,
        const std::vector<VkDescriptorSetLayout> usedLayouts);
    ~RenderPass();

    const VkRenderPass& renderPass() const;
    const Pipeline& pipeline() const;
    const VkExtent2D& extent() const;
private:
    const VkContext* m_context;
    const VkExtent2D m_extent;

    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    Pipeline* m_pipeline = nullptr;
};