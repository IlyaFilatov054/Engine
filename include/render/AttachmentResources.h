#pragma once

#include "render/DescriptorManager.h"
#include "render/StagedBuffer.h"
#include "render/VkContext.h"
#include "render/Image.h"
#include <cstdint>
#include <glm/ext/scalar_uint_sized.hpp>
#include <map>
#include <vector>
#include <vulkan/vulkan_core.h>

typedef uint32_t ImageAttachmentHandle;
struct ImageAttachment {  
    Image* image;
    bool external;
};

typedef uint32_t ReadAttachmentHandle;
struct ReadAttachment {
    DescriptorSetLayoutHandle descriptorSetLayoutHandle;
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
    DescriptorSetLayoutHandle descriptorSetLayoutHandle;
    VkDescriptorSet descriptor;
    StagedBuffer* buffer;
};

class AttachmentResources {
public:
    AttachmentResources(const VkContext* context);
    ~AttachmentResources();

    ImageAttachment imageAttachment(ImageAttachmentHandle handle) const;
    void addImageAttachment(ImageAttachmentHandle handle, Image* attachment);
    void addImageAttachment(
        ImageAttachmentHandle handle,
        VkFormat format,
        VkImageUsageFlags usage,
        VkExtent3D extent,
        VkImageAspectFlags aspect
    );

    WriteAttachment writeAttachment(WriteAttachmentHandle handle) const;
    void addWriteAttachment(
        WriteAttachmentHandle handle,
        const VkRenderPass renderPass,
        const VkExtent2D& extent,
        const std::vector<ImageAttachmentHandle>& imageAttachments
    );

    ReadAttachment readAttachment(ReadAttachmentHandle handle) const;
    void addReadAttachment(
        ReadAttachmentHandle handle,
        DescriptorSetLayoutHandle descriptorSetLayoutHandle,
        VkDescriptorSet descriptor,
        const std::vector<ImageAttachmentHandle>& imageAttachments
    );

    DescriptorAttachment descriptorAttachment(DescriptorAttachmentHandle handle) const;
    void addDescriptorAttachment(
        DescriptorAttachmentHandle handle,
        DescriptorSetLayoutHandle descriptorSetLayoutHandle,
        const VkDescriptorSet descriptor,
        const VkDescriptorType type,
        const VkBufferUsageFlagBits usage,
        uint32_t bufferSize
    );
    void addDescriptorAttachment(
        DescriptorAttachmentHandle handle,
        const VkDescriptorSet descriptor,
        const VkDescriptorType type
    );
private:
    const VkContext* m_context;

    std::map<ImageAttachmentHandle, ImageAttachment> m_imageAttachments;
    std::map<WriteAttachmentHandle, WriteAttachment> m_writeAttachments;
    std::map<ReadAttachmentHandle, ReadAttachment> m_readAttachments;
    std::map<DescriptorAttachmentHandle, DescriptorAttachment> m_descriptorAttachments;

    VkSampler m_readAttachmentSampler = VK_NULL_HANDLE;
};