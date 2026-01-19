#pragma once

#include "render/ShaderManager.h"
#include "render/Swapchain.h"
#include "render/VkContext.h"
#include <vector>
#include <vulkan/vulkan_core.h>

class Pipeline {
public:
    Pipeline(const VkContext* context, const Swapchain* swapchain, 
        const VkRenderPass renderPass, const ShaderManager* shaderManager,
        const std::vector<VkDescriptorSetLayout> usedLayouts);
    ~Pipeline();
    
    const VkPipelineLayout& layout() const;
    const VkPipeline& pipeline() const;
private:
    const VkContext* m_context;
    const Swapchain* m_swapchain;
    const VkRenderPass m_renderPass;
    const ShaderManager* m_shaderManager;

    VkPipelineLayout m_layout = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
};