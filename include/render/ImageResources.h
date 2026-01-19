#pragma once

#include "render/Image.h"
#include "render/VkContext.h"
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>

struct ImageAttachment {  
    Image* image;
    bool external;
};

class ImageResources {
public:
    ImageResources(const VkContext* context);
    ~ImageResources();

    const VkFramebuffer& framebuffer() const;
    const VkSemaphore& renderFinishedSemaphore() const;

    uint32_t addAttachment(Image* attachment);
    uint32_t addAttachment(VkFormat format, VkImageUsageFlags usage, VkExtent3D extent, VkImageAspectFlags aspect);

private:
    const VkContext* m_context;

    std::vector<ImageAttachment> m_attachments;
    VkSemaphore m_renderFinished = VK_NULL_HANDLE;

    void createSemaphore();
};