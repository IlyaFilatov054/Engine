#include "render/AttachmentResources.h"
#include <cstdint>
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
        vkDestroyFramebuffer(m_context->device(), a.framebuffer, nullptr);
    }
    for(auto& a : m_imageAttachments) {
        if(a.external) continue;
        delete a.image;
    }
    vkDestroySampler(m_context->device(), m_readAttachmentSampler, nullptr);
}

ImageAttachment AttachmentResources::imageAttachment(uint32_t id) const {
    return m_imageAttachments[id];
}

uint32_t AttachmentResources::addImageAttachment(Image* attachment) {
    m_imageAttachments.emplace_back(attachment, true);
    return m_imageAttachments.size() - 1;
}

uint32_t AttachmentResources::addImageAttachment(VkFormat format, VkImageUsageFlags usage, VkExtent3D extent, VkImageAspectFlags aspect) {
    Image* attachment = new Image(m_context, format);
    attachment->createImage(usage, extent);
    attachment->createView(aspect);
    m_imageAttachments.emplace_back(attachment, false);
    return m_imageAttachments.size() - 1;
}

WriteAttachment AttachmentResources::writeAttachment(uint32_t id) const{
    return m_writeAttachments[id];
}

uint32_t AttachmentResources::addWriteAttachment(
    const VkRenderPass renderPass,
    const VkExtent2D& extent,
    const std::vector<uint32_t>& attachments
) {
    std::vector<VkImageView> framebufferAttachments;
    for(const auto& i : attachments) {
        framebufferAttachments.push_back(imageAttachment(i).image->view());
    }

    WriteAttachment attachment {
        .framebuffer = VK_NULL_HANDLE,
        .images = attachments
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
    
    m_writeAttachments.push_back(attachment);
    return m_writeAttachments.size() - 1;
}

ReadAttachment AttachmentResources::readAttachment(uint32_t id) const {
    return m_readAttachments[id];
}

uint32_t AttachmentResources::addReadAttachment(
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
        .descriptor = descriptor,
        .images = attachments
    };
    m_readAttachments.push_back(attachment);
    return m_readAttachments.size() - 1;
}