#include "render/ImageResources.h"
#include "render/Image.h"
#include "render/VkUtils.h"
#include <vulkan/vulkan_core.h>

ImageResources::ImageResources(const VkContext* context) : m_context(context) {
    createSemaphore();
}

ImageResources::~ImageResources() {
    vkDestroySemaphore(m_context->device(), m_renderFinished, nullptr);
    for(auto& a : m_attachments) {
        if(a.external) continue;
        delete a.image;
    }
}

const VkSemaphore& ImageResources::renderFinishedSemaphore() const {
    return m_renderFinished;
}

void ImageResources::createSemaphore() {
    VkSemaphoreCreateInfo semaphoreCreateInfo {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };
    auto res = vkCreateSemaphore(m_context->device(), &semaphoreCreateInfo, nullptr, &m_renderFinished);
    validateVkResult(res, "vkCreateSemaphore");
}

uint32_t ImageResources::addAttachment(Image* attachment) {
    m_attachments.emplace_back(attachment, true);
    return m_attachments.size() - 1;
}

uint32_t ImageResources::addAttachment(VkFormat format, VkImageUsageFlags usage, VkExtent3D extent, VkImageAspectFlags aspect) {
    Image* attachment = new Image(m_context, format);
    attachment->createImage(usage, extent);
    attachment->createView(aspect);
    m_attachments.emplace_back(attachment, false);
    return m_attachments.size() - 1;
}