#pragma once
#include "render/VkContext.h"
#include "render/Swapchain.h"
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "render/FrameSyncObject.h"

class RenderCore {
public:
    RenderCore(const VkContext* context, const Swapchain* swapchain);
    ~RenderCore();

    void drawFrame();

private:
    const VkContext* m_context = nullptr;
    const Swapchain* m_swapchain = nullptr;
    
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_framebuffers;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    std::vector<VkShaderModule> m_shaders;
    
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> m_commandBuffers;
    
    uint32_t m_maxFramesInFlight;
    std::vector<FrameSyncObject> m_syncObjects;
    std::vector<FrameSyncObject*> m_attachedSyncObjects;
    uint32_t m_currentFrame = 0;

    void createRenderPass();
    void createFramebuffers();
    VkShaderModule createShaderModule(const std::vector<char> code);
    void createPipeline();
    void createCommandPool();
    void createCommandBuffers();
    void initSync();
    [[deprecated("ВРЕМЕННО")]] void recordCommandBuffers();
};