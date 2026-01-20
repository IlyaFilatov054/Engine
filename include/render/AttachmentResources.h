#pragma once

#include "render/VkContext.h"
#include "render/Image.h"
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>

struct ImageAttachment {  
    Image* image;
    bool external;
};

struct ReadAttachment {
    VkDescriptorSet descriptor;
    std::vector<uint32_t> images;
};

struct WriteAttachment {
    VkFramebuffer framebuffer;
    std::vector<uint32_t> images;
};

class AttachmentResources {
public:
    AttachmentResources(const VkContext* context);
    ~AttachmentResources();

    ImageAttachment imageAttachment(uint32_t id) const;
    uint32_t addImageAttachment(Image* attachment);
    uint32_t addImageAttachment(
        VkFormat format,
        VkImageUsageFlags usage,
        VkExtent3D extent,
        VkImageAspectFlags aspect
    );

    WriteAttachment writeAttachment(uint32_t id) const;
    uint32_t addWriteAttachment(
        const VkRenderPass renderPass,
        const VkExtent2D& extent,
        const std::vector<uint32_t>& attachments
    );

    ReadAttachment readAttachment(uint32_t id) const;
    uint32_t addReadAttachment(
        VkDescriptorSet descriptor,
        const std::vector<uint32_t>& attachments
    );
private:
    const VkContext* m_context;

    std::vector<ImageAttachment> m_imageAttachments;
    std::vector<WriteAttachment> m_writeAttachments;
    std::vector<ReadAttachment> m_readAttachments;

    VkSampler m_readAttachmentSampler = VK_NULL_HANDLE;
};