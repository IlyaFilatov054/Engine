#pragma once

#include "render/VkContext.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

const uint32_t SSBO_SIZE = 1024;

class FrameResources {
public:
    FrameResources(const VkContext* context);
    ~FrameResources();
    
    const VkSemaphore& imageAcquiredSemaphore() const;
    const VkCommandBuffer& commandBuffer() const;
    const VkFence& submitFence() const;
    const void waitFence() const;
private:
    const VkContext* m_context;
    
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
    VkSemaphore m_imageAcquired = VK_NULL_HANDLE;
    VkFence m_submited = VK_NULL_HANDLE;

    void createCommandPool();
    void createCommandBuffer();
    void createSynchronization();
};