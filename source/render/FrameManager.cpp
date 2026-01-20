#include "render/FrameManager.h"
#include "render/AttachmentResources.h"
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
    for(const auto& r : m_attachmentResources) delete r;
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

void FrameManager::createAttachmentResources() {
    for(uint32_t i = 0; i < m_imageCount; i++) {
        m_attachmentResources.push_back(new AttachmentResources(m_context));
    }
}

void FrameManager::nextFrame() {
    m_currentFrame = (m_currentFrame + 1) % m_maxFrames;
}

const FrameResources* FrameManager::currentFrameResources() const {
    return m_frameResources[m_currentFrame];
}

const AttachmentResources* FrameManager::attachmentResources(const uint32_t image) const {
    return m_attachmentResources[image];
}

const ImageResources* FrameManager::imageResources(const uint32_t image) const {
    return m_imageResources[image];
}

const uint32_t FrameManager::maxFrames() const {
    return m_maxFrames;
}

uint32_t FrameManager::addImageAttachments(const std::vector<Image*>& images) {
    uint32_t attachment;
    for(uint32_t i = 0; i < m_imageCount; i++) {
        attachment = m_attachmentResources[i]->addImageAttachment(images[i]);
    }
    return attachment;
}

uint32_t FrameManager::addImageAttachment
(
    VkFormat format,
    VkImageUsageFlags usage,
    VkExtent3D extent,
    VkImageAspectFlags aspect
) {
    uint32_t attachment;
    for(auto& i : m_attachmentResources) {
        attachment = i->addImageAttachment(format, usage, extent, aspect);
    }
    return attachment;
}

uint32_t FrameManager::addWriteAttachment
(
    const VkRenderPass renderPass,
    const VkExtent2D& extent,
    const std::vector<uint32_t>& attachments
) {
    uint32_t attachment;
    for(auto& i : m_attachmentResources) {
        attachment = i->addWriteAttachment(renderPass, extent, attachments);
    }
    return attachment;
}

uint32_t FrameManager::addReadAttachment
(
    VkDescriptorSet descriptor,
    const std::vector<uint32_t>& attachments
) {
    uint32_t attachment;
    for(auto& i : m_attachmentResources) {
        attachment = i->addReadAttachment(descriptor, attachments);
    }
    return attachment;
}