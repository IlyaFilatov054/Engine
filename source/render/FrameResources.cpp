#include "render/FrameResources.h"
#include <vulkan/vulkan_core.h>
#include "render/VkUtils.h"

FrameResources::FrameResources(const VkContext* context, const VkCommandPool pool) : m_context(context), m_commadPool(pool) {
    createCommandBuffer();
    createSynchronization();
}

FrameResources::~FrameResources() {
    vkDestroySemaphore(m_context->device(), m_imageAcquired, nullptr);
    vkDestroyFence(m_context->device(), m_submited, nullptr);
    vkFreeCommandBuffers(m_context->device(), m_commadPool, 1, &m_commandBuffer);
}

const VkSemaphore& FrameResources::imageAcquiredSemaphore() const {
    return m_imageAcquired;
}

const VkCommandBuffer& FrameResources::commandBuffer() const {
    return m_commandBuffer;
}

const VkFence& FrameResources::submitFence() const {
    return m_submited;
}

const void FrameResources::waitFence() const {
    vkWaitForFences(m_context->device(), 1, &m_submited, VK_TRUE, UINT64_MAX);
    vkResetFences(m_context->device(), 1, &m_submited);
}

void FrameResources::createCommandBuffer() {
    VkCommandBufferAllocateInfo commandBufferInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_commadPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    vkAllocateCommandBuffers(m_context->device(), &commandBufferInfo, &m_commandBuffer);
}

void FrameResources::createSynchronization() {
    VkSemaphoreCreateInfo semaphoreCreateInfo {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };
    VkFenceCreateInfo fenceInfo {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };
    auto res = vkCreateSemaphore(m_context->device(), &semaphoreCreateInfo, nullptr, &m_imageAcquired);
    validateVkResult(res, "vkCreateSemaphore");
    res = vkCreateFence(m_context->device(), &fenceInfo, nullptr, &m_submited);
    validateVkResult(res, "vkCreateFence");
}