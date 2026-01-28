#include "render/AttachmentResources.h"
#include "render/StagedBuffer.h"
#include <cstdint>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

AttachmentResources::AttachmentResources(const VkContext* context) : m_context(context) {
    VkSamplerCreateInfo samplerInfo {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_NEAREST,
        .minFilter = VK_FILTER_NEAREST,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .anisotropyEnable = VK_FALSE,
        .maxAnisotropy = 0,
        .compareEnable = VK_FALSE,
        .unnormalizedCoordinates = VK_FALSE,
    };
    vkCreateSampler(m_context->device(), &samplerInfo, nullptr, &m_readAttachmentSampler);
}

AttachmentResources::~AttachmentResources() {
    for(auto& a : m_writeAttachments) {
        vkDestroyFramebuffer(m_context->device(), a.second.framebuffer, nullptr);
    }
    for(auto& a : m_imageAttachments) {
        if(a.second.external) continue;
        delete a.second.image;
    }
    for(auto& a : m_descriptorAttachments) {
        delete a.second.buffer;
    }
    vkDestroySampler(m_context->device(), m_readAttachmentSampler, nullptr);
}

ImageAttachment AttachmentResources::imageAttachment(ImageAttachmentHandle handle) const {
    return m_imageAttachments.at(handle);
}

void AttachmentResources::addImageAttachment(ImageAttachmentHandle handle, Image* attachment) {
    if(m_imageAttachments.contains(handle)) throw std::runtime_error("Image attachment handle already present!");
    m_imageAttachments[handle] = {.image = attachment, .external = true};
}

void AttachmentResources::addImageAttachment(ImageAttachmentHandle handle, VkFormat format, VkImageUsageFlags usage, VkExtent3D extent, VkImageAspectFlags aspect) {
    Image* attachment = new Image(m_context, format);
    attachment->createImage(usage, extent);
    attachment->createView(aspect);
    if(m_imageAttachments.contains(handle)) throw std::runtime_error("Image attachment handle already present!");
    m_imageAttachments[handle] = {.image = attachment, .external = false};
}

WriteAttachment AttachmentResources::writeAttachment(WriteAttachmentHandle handle) const{
    return m_writeAttachments.at(handle);
}

void AttachmentResources::addWriteAttachment(
    WriteAttachmentHandle handle,
    const VkRenderPass renderPass,
    const VkExtent2D& extent,
    const std::vector<ImageAttachmentHandle>& imageAttachments
) {
    std::vector<VkImageView> framebufferAttachments;
    for(const auto& i : imageAttachments) {
        framebufferAttachments.push_back(imageAttachment(i).image->view());
    }

    WriteAttachment attachment {
        .framebuffer = VK_NULL_HANDLE,
        .images = imageAttachments
    };
    
    VkFramebufferCreateInfo framebufferInfo {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = renderPass,
        .attachmentCount = static_cast<uint32_t>(framebufferAttachments.size()),
        .pAttachments = framebufferAttachments.data(),
        .width = extent.width,
        .height = extent.height,
        .layers = 1
    };
    vkCreateFramebuffer(m_context->device(), &framebufferInfo, nullptr, &attachment.framebuffer);
    
    if(m_writeAttachments.contains(handle)) throw std::runtime_error("Write attachment handle already present!");
    m_writeAttachments[handle] = attachment;
}

ReadAttachment AttachmentResources::readAttachment(ReadAttachmentHandle handle) const {
    return m_readAttachments.at(handle);
}

void AttachmentResources::addReadAttachment(
    ReadAttachmentHandle handle,
    DescriptorSetLayoutHandle descriptorSetLayoutHandle,
    VkDescriptorSet descriptor,
    const std::vector<uint32_t>& attachments
) {
    for(uint32_t i = 0; i < attachments.size(); i++) {
        const auto* image = imageAttachment(attachments[i]).image;
        VkDescriptorImageInfo info {
            .sampler = m_readAttachmentSampler,
            .imageView = image->view(),
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
        VkWriteDescriptorSet write {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptor,
            .dstBinding = i,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &info
        };
        vkUpdateDescriptorSets(m_context->device(), 1, &write, 0, nullptr);
    }
    ReadAttachment attachment {
        .descriptorSetLayoutHandle = descriptorSetLayoutHandle,
        .descriptor = descriptor,
        .images = attachments
    };

    if(m_readAttachments.contains(handle)) throw std::runtime_error("Read attachment handle already present!");
    m_readAttachments[handle] = attachment;
}

DescriptorAttachment AttachmentResources::descriptorAttachment(DescriptorAttachmentHandle handle) const {
    return m_descriptorAttachments.at(handle);
}

void AttachmentResources::addDescriptorAttachment(
    DescriptorAttachmentHandle handle,
    DescriptorSetLayoutHandle descriptorSetLayoutHandle,
    const VkDescriptorSet descriptor,
    const VkDescriptorType type,
    const VkBufferUsageFlagBits usage,
    uint32_t bufferSize
) {
    DescriptorAttachment attachment {
        .descriptorSetLayoutHandle = descriptorSetLayoutHandle,
        .descriptor = descriptor,
        .buffer = new StagedBuffer(m_context, bufferSize, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
    };
    VkDescriptorBufferInfo bufferInfo {
        .buffer = attachment.buffer->buffer(),
        .offset = 0,
        .range = bufferSize,
    };
    VkWriteDescriptorSet write {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptor,
        .dstBinding = 0,
        .descriptorCount = 1,
        .descriptorType = type,
        .pBufferInfo = &bufferInfo,
    };
    vkUpdateDescriptorSets(m_context->device(), 1, &write, 0, nullptr);

    if(m_descriptorAttachments.contains(handle)) throw std::runtime_error("Descriptor attachment handle already present!");
    m_descriptorAttachments[handle] = attachment;   
}