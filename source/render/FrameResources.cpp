#include "render/FrameResources.h"
#include <vulkan/vulkan_core.h>
#include "render/VkUtils.h"

FrameResources::FrameResources(const VkContext* context) : m_context(context) {
    createCommandPool();
    createCommandBuffer();
    createSynchronization();
}

FrameResources::~FrameResources() {
    vkDestroySemaphore(m_context->device(), m_imageAcquired, nullptr);
    vkDestroyFence(m_context->device(), m_submited, nullptr);
    vkFreeCommandBuffers(m_context->device(), m_commandPool, 1, &m_commandBuffer);
    vkDestroyCommandPool(m_context->device(), m_commandPool, nullptr);
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

void FrameResources::createCommandPool() {
    VkCommandPoolCreateInfo commandPoolInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = m_context->graphicsQueueIndex(),
    };
    vkCreateCommandPool(m_context->device(), &commandPoolInfo, nullptr, &m_commandPool);
};

void FrameResources::createCommandBuffer() {
    VkCommandBufferAllocateInfo commandBufferInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_commandPool,
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