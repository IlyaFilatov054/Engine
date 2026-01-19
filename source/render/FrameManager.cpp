#include "render/FrameManager.h"
#include "render/FrameResources.h"
#include "render/ImageResources.h"
#include <algorithm>
#include <cstdint>

FrameManager::FrameManager(const VkContext* context, uint32_t imageCount) :
m_context(context), m_maxFrames(std::min(2u, imageCount)), m_imageCount(imageCount) {
}

FrameManager::~FrameManager() {
    for(const auto& r : m_imageResources) delete r;
    for(const auto& r : m_frameResources) delete r;
}

void FrameManager::createImageResources() {
    for(uint32_t i = 0; i < m_imageCount; i++) {
        m_imageResources.push_back(new ImageResources(m_context));   
    }
}

void FrameManager::createFrameResources(const std::vector<VkDescriptorSet>& sets) {
    for(uint32_t i = 0; i < m_maxFrames; i++) {
        m_frameResources.push_back(new FrameResources(m_context, sets[i]));
    }
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

const uint32_t FrameManager::maxFrames() const {
    return m_maxFrames;
}

uint32_t FrameManager::addImageAttachments(const std::vector<Image*>& images) const {
    uint32_t attachment;
    for(uint32_t i = 0; i < m_imageCount; i++) {
        attachment = m_imageResources[i]->addAttachment(images[i]);
    }
    return attachment;
}

uint32_t FrameManager::addImageAttachment(VkFormat format, VkImageUsageFlags usage, VkExtent3D extent, VkImageAspectFlags aspect) const {
    uint32_t attachment;
    for(const auto& r : m_imageResources) {
        attachment = r->addAttachment(format, usage, extent, aspect);
    }
    return attachment;
}