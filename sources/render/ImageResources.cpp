#include "render/ImageResources.h"
#include "render/VkUtils.h"
#include <vulkan/vulkan_core.h>

ImageResources::ImageResources(const VkContext* context) : m_context(context) {
    createSemaphore();
}

ImageResources::~ImageResources() {
    vkDestroySemaphore(m_context->device(), m_renderFinished, nullptr);
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

