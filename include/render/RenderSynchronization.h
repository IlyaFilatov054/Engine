#pragma once
#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "render/VkContext.h"

class RenderSynchronization {
public:
    RenderSynchronization(const VkContext* context, uint32_t imageCount);
    ~RenderSynchronization();

    void nextFrame();
    const VkFence& gpuReadyFence() const;
    const VkSemaphore& imageAvailable() const; 
    const VkSemaphore& renderFinishedSemaphore(uint32_t imageIndex) const;
    const uint32_t currentFrame() const;
    const uint32_t maxFrames() const;
    void waitCurrentFence() const;
private:
    std::vector<VkFence> m_gpuReady;
    std::vector<VkSemaphore> m_imageAvailable;
    std::vector<VkSemaphore> m_renderFinished;
    uint32_t m_maxFrames;
    uint32_t m_currentFrame = 0;
    const VkContext* m_context;
};