#include "render/ImageResources.h"
#include "render/Image.h"
#include "render/Swapchain.h"
#include "render/VkUtils.h"
#include <vulkan/vulkan_core.h>

ImageResources::ImageResources(const VkContext* context, const Swapchain* swapchain, const VkImageView imageView, const VkRenderPass renderPass) : 
m_context(context),
m_swapchain(swapchain),
m_imageView(imageView),
m_renderPass(renderPass) {
    m_depthImage = new Image(m_context);
    m_depthImage->createImage(VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, {.width = swapchain->extent().width, .height = swapchain->extent().height, .depth = 1});
    m_depthImage->createView(VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT);
    createFramebuffer();
    createSemaphore();
}

ImageResources::~ImageResources() {
    vkDestroySemaphore(m_context->device(), m_renderFinished, nullptr);
    vkDestroyFramebuffer(m_context->device(), m_framebuffer, nullptr);
    delete m_depthImage;
}

const VkFramebuffer& ImageResources::framebuffer() const {
    return m_framebuffer;
}

const VkSemaphore& ImageResources::renderFinishedSemaphore() const {
    return m_renderFinished;
}

void ImageResources::createFramebuffer() {
    VkImageView attachments[] = {m_imageView, m_depthImage->view()};
    VkFramebufferCreateInfo framebufferInfo {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = m_renderPass,
        .attachmentCount = 2,
        .pAttachments = attachments,
        .width = m_swapchain->extent().width,
        .height = m_swapchain->extent().height,
        .layers = 1,
    };
    auto res = vkCreateFramebuffer(m_context->device(), &framebufferInfo, nullptr, &m_framebuffer);
    validateVkResult(res, "vkCreateFramebuffer");
}

void ImageResources::createSemaphore() {
    VkSemaphoreCreateInfo semaphoreCreateInfo {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };
    auto res = vkCreateSemaphore(m_context->device(), &semaphoreCreateInfo, nullptr, &m_renderFinished);
    validateVkResult(res, "vkCreateSemaphore");
}