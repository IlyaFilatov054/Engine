#pragma once

#include "render/AttachmentResources.h"
#include "render/FrameResources.h"
#include "render/ImageResources.h"
#include "render/VkContext.h"
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>

class FrameManager {
public:
    FrameManager(const VkContext* context, uint32_t imageCount);
    ~FrameManager();

    void createImageResources();
    void createFrameResources();
    void createAttachmentResources();

    void nextFrame();
    const FrameResources* currentFrameResources() const;
    AttachmentResources* attachmentResources(const uint32_t image) const;
    const ImageResources* imageResources(const uint32_t image) const;
   
    const uint32_t maxFrames() const;
    const uint32_t imageCount() const;
private:
    const VkContext* m_context;

    std::vector<ImageResources*> m_imageResources;
    const uint32_t m_imageCount;

    const uint32_t m_maxFrames;
    std::vector<FrameResources*> m_frameResources;
    uint32_t m_currentFrame = 0;

    std::vector<AttachmentResources*> m_attachmentResources;
};