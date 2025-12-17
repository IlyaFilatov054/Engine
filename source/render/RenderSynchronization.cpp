#include "render/RenderSynchronization.h"
#include "render/VkUtils.h"
#include <vulkan/vulkan_core.h>

RenderSynchronization::RenderSynchronization(const VkContext* context, uint32_t imageCount) : 
m_context(context) {
    m_maxFrames = std::min(2u, imageCount);
    
    m_gpuReady.resize(m_maxFrames);
    m_imageAvailable.resize(m_maxFrames);
    m_renderFinished.resize(imageCount);

    VkSemaphoreCreateInfo semaphoreCreateInfo {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };
    VkFenceCreateInfo fenceInfo {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    VkResult res;
    for(uint32_t i = 0; i < m_maxFrames; i++) {
        res = vkCreateSemaphore(m_context->device(), &semaphoreCreateInfo, nullptr, &m_imageAvailable[i]);
        validateVkResult(res, "vkCreateSemaphore");
        res = vkCreateFence(m_context->device(), &fenceInfo, nullptr, &m_gpuReady[i]);
        validateVkResult(res, "vkCreateFence");
    }
    for(uint32_t i = 0; i < imageCount; i++) {
        res = vkCreateSemaphore(m_context->device(), &semaphoreCreateInfo, nullptr, &m_renderFinished[i]);
        validateVkResult(res, "vkCreateSemaphore");
    }
}

RenderSynchronization::~RenderSynchronization() {
    for(uint32_t i = 0; i < m_maxFrames; i++) {
        vkDestroySemaphore(m_context->device(), m_imageAvailable[i], nullptr);
        vkDestroyFence(m_context->device(), m_gpuReady[i], nullptr);
    }
    for(uint32_t i = 0; i < m_renderFinished.size(); i++) {
        vkDestroySemaphore(m_context->device(), m_renderFinished[i], nullptr);
    }
}

void RenderSynchronization::nextFrame() {
    m_currentFrame = (m_currentFrame + 1) % m_maxFrames;
}

const VkFence& RenderSynchronization::gpuReadyFence() const {
    return m_gpuReady[m_currentFrame];
}

const VkSemaphore& RenderSynchronization::imageAvailable() const {
    return m_imageAvailable[m_currentFrame];
}

const VkSemaphore& RenderSynchronization::renderFinishedSemaphore(uint32_t imageIndex) const {
    return m_renderFinished[imageIndex];
}

const uint32_t RenderSynchronization::currentFrame() const {
    return m_currentFrame;
}

const uint32_t RenderSynchronization::maxFrames() const {
    return m_maxFrames;
}

void RenderSynchronization::waitCurrentFence() const {
    vkWaitForFences(m_context->device(), 1, &gpuReadyFence(), VK_TRUE, UINT64_MAX);
    vkResetFences(m_context->device(), 1, &gpuReadyFence());
}