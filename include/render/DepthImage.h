#pragma once

#include "render/Swapchain.h"
#include "render/VkContext.h"
#include <vulkan/vulkan_core.h>

class DepthImage {
public:
    DepthImage(const VkContext* context, const Swapchain* swapchain);
    ~DepthImage();

    const VkImageView& view() const;
private:
    VkImage m_image = VK_NULL_HANDLE;
    VkImageView m_imageView = VK_NULL_HANDLE;
    VkDeviceMemory m_imageMemory = VK_NULL_HANDLE;

    const VkContext* m_context;
};