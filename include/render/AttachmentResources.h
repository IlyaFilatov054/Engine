#pragma once

#include "render/StagedBuffer.h"
#include "render/VkContext.h"
#include "render/Image.h"
#include <cstdint>
#include <glm/ext/scalar_uint_sized.hpp>
#include <vector>
#include <vulkan/vulkan_core.h>

typedef uint32_t ImageAttachmentHandle;
struct ImageAttachment {  
    Image* image;
    bool external;
};

typedef uint32_t ReadAttachmentHandle;
struct ReadAttachment {
    VkDescriptorSet descriptor;
    std::vector<uint32_t> images;
};

typedef uint32_t WriteAttachmentHandle;
struct WriteAttachment {
    VkFramebuffer framebuffer;
    std::vector<uint32_t> images;
};

typedef uint32_t DescriptorAttachmentHandle;
struct DescriptorAttachment {
    VkDescriptorSet descriptor;
    StagedBuffer* buffer;
};

class AttachmentResources {
public:
    AttachmentResources(const VkContext* context);
    ~AttachmentResources();

    ImageAttachment imageAttachment(ImageAttachmentHandle handle) const;
    ImageAttachmentHandle addImageAttachment(Image* attachment);
    ImageAttachmentHandle addImageAttachment(
        VkFormat format,
        VkImageUsageFlags usage,
        VkExtent3D extent,
        VkImageAspectFlags aspect
    );

    WriteAttachment writeAttachment(WriteAttachmentHandle handle) const;
    WriteAttachmentHandle addWriteAttachment(
        const VkRenderPass renderPass,
        const VkExtent2D& extent,
        const std::vector<ImageAttachmentHandle>& imageAttachments
    );

    ReadAttachment readAttachment(ReadAttachmentHandle handle) const;
    ReadAttachmentHandle addReadAttachment(
        VkDescriptorSet descriptor,
        const std::vector<ImageAttachmentHandle>& imageAttachments
    );

    DescriptorAttachment descriptorAttachment(DescriptorAttachmentHandle handle) const;
    DescriptorAttachmentHandle addDescriptorAttachment(
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