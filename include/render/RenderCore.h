#pragma once
#include "render/VkContext.h"
#include "render/Swapchain.h"
#include <vector>
#include <vulkan/vulkan_core.h>

class RenderCore {
public:
    RenderCore(const VkContext* context, const Swapchain* swapchain);
    ~RenderCore();

private:
    const VkContext* m_context = nullptr;
    const Swapchain* m_swapchain = nullptr;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_framebuffers;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;

    void createRenderPass();
    void createFramebuffers();
    void createPipeline();
    VkShaderModule createShaderModule(const std::vector<char> code);
};