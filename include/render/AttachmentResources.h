#pragma once

#include "render/StagedBuffer.h"
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

struct DescriptorAttachment {
    VkDescriptorSet descriptor;
    StagedBuffer* buffer;
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

    const WriteAttachment& writeAttachment(uint32_t id) const;
    uint32_t addWriteAttachment(
        const VkRenderPass renderPass,
        const VkExtent2D& extent,
        const std::vector<uint32_t>& attachments
    );

    const ReadAttachment& readAttachment(uint32_t id) const;
    uint32_t addReadAttachment(
        VkDescriptorSet descriptor,
        const std::vector<uint32_t>& attachments
    );

    const DescriptorAttachment& descriptorAttachment(uint32_t id) const;
    uint32_t addDescriptorAttachment(
        const VkDescriptorSet descriptor,
        const VkDescriptorType type,
        const VkBufferUsageFlagBits usage,
        uint32_t bufferSize
    );
private:
    const VkContext* m_context;

    std::vector<ImageAttachment> m_imageAttachments;
    std::vector<WriteAttachment> m_writeAttachments;
    std::vector<ReadAttachment> m_readAttachments;
    std::vector<DescriptorAttachment> m_descriptorAttachments;

    VkSampler m_readAttachmentSampler = VK_NULL_HANDLE;
};