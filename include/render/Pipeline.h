#pragma once

#include "render/VkContext.h"
#include <vector>
#include <vulkan/vulkan_core.h>

struct ShaderDescription {
    VkShaderModule module;
    VkShaderStageFlagBits stage;
};

class Pipeline {
public:
    Pipeline(const VkContext* context, const VkExtent2D& extent, 
        const VkRenderPass renderPass, const std::vector<ShaderDescription>& shaders,
        const std::vector<VkDescriptorSetLayout> usedLayouts);
    ~Pipeline();
    
    const VkPipelineLayout& layout() const;
    const VkPipeline& pipeline() const;
private:
    const VkContext* m_context;

    VkPipelineLayout m_layout = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
};