#pragma once

#include "render/VkContext.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

class FrameResources {
public:
    FrameResources(const VkContext* context, const VkCommandPool pool);
    ~FrameResources();
    
    const VkSemaphore& imageAcquiredSemaphore() const;
    const VkCommandBuffer& commandBuffer() const;
    const VkFence& submitFence() const;
    const void waitFence() const;
private:
    const VkContext* m_context;
    const VkCommandPool m_commadPool;

    VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
    VkSemaphore m_imageAcquired = VK_NULL_HANDLE;
    VkFence m_submited = VK_NULL_HANDLE;

    void createCommandBuffer();
    void createSynchronization();
};