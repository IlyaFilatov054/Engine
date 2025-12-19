#pragma once

#include "render/FrameResources.h"
#include "render/ImageResources.h"
#include "render/Swapchain.h"
#include "render/VkContext.h"
#include <cstdint>
#include <vector>

class FrameManager {
public:
    FrameManager(const VkContext* context, const Swapchain* swapchain, const VkCommandPool pool, const VkRenderPass renderPass);
    ~FrameManager();

    void nextFrame();
    const FrameResources& currentFrameResources() const;
    const ImageResources& imageResources(const uint32_t image) const;
private:
    const VkContext* m_context;
    const Swapchain* m_swapchain;
    const VkCommandPool m_commandPool;
    const VkRenderPass m_renderPass;

    std::vector<ImageResources*> m_imageResources;
    std::vector<FrameResources*> m_frameResources;

    const uint32_t m_maxFrames;
    uint32_t m_currentFrame = 0;
};