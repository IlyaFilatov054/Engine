#pragma once

#include "render/DescriptorManager.h"
#include "render/Pipeline.h"
#include "render/ShaderManager.h"
#include "render/Swapchain.h"
#include "render/VkContext.h"
#include <vulkan/vulkan_core.h>

class RenderPass {
public:
    RenderPass(const VkContext* context, const Swapchain* swapchain, 
        const ShaderManager* shaderManager, const DescriptorManager* descriptorManager);
    ~RenderPass();

    const VkRenderPass& renderPass() const;
    const Pipeline& pipeline() const;
private:
    const VkContext* m_context;
    const Swapchain* m_swapchain;

    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    Pipeline* m_pipeline = nullptr;
};