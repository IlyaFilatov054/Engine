#include "render/FrameManager.h"
#include "render/FrameResources.h"
#include "render/ImageResources.h"
#include <algorithm>
#include <cstdint>

FrameManager::FrameManager(const VkContext* context, const Swapchain* swapchain, const VkRenderPass renderPass) :
m_context(context),
m_swapchain(swapchain),
m_renderPass(renderPass),
m_maxFrames(std::min(2u, (uint32_t)swapchain->imageViews().size())) {
    for(const auto& i : m_swapchain->imageViews()) m_imageResources.push_back(new ImageResources(m_context, m_swapchain, i, m_renderPass));
    for(uint32_t i = 0; i < m_maxFrames; i++) m_frameResources.push_back(new FrameResources(m_context));
}

FrameManager::~FrameManager() {
    for(const auto& r : m_imageResources) delete r;
    for(const auto& r : m_frameResources) delete r;
}

void FrameManager::nextFrame() {
    m_currentFrame = (m_currentFrame + 1) % m_maxFrames;
}

const FrameResources& FrameManager::currentFrameResources() const {
    return *m_frameResources[m_currentFrame];
}

const ImageResources& FrameManager::imageResources(const uint32_t image) const {
    return *m_imageResources[image];
}