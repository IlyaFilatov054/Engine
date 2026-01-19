#pragma once

#include "render/DescriptorManager.h"
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
    void createFrameResources(const std::vector<VkDescriptorSet>& sets);

    void nextFrame();
    const FrameResources& currentFrameResources() const;
    const ImageResources& imageResources(const uint32_t image) const;
    const uint32_t maxFrames() const;

    uint32_t addImageAttachments(const std::vector<Image*>& images) const;
    uint32_t addImageAttachment(VkFormat format, VkImageUsageFlags usage, VkExtent3D extent, VkImageAspectFlags aspect) const;
private:
    const VkContext* m_context;

    std::vector<ImageResources*> m_imageResources;
    const uint32_t m_imageCount;

    const uint32_t m_maxFrames;
    std::vector<FrameResources*> m_frameResources;
    uint32_t m_currentFrame = 0;
};