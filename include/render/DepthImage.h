#pragma once

#include "render/Swapchain.h"
#include "render/VkContext.h"
#include "vulkan/vulkan.h"

class DepthImage {
public:
    DepthImage(const VkContext* context, const Swapchain* swapchain);
    ~DepthImage();

    const VkImageView& view() const;
private:
    VkImage m_image;
    VkImageView m_imageView;
    VkDeviceMemory m_imageMemory;

    const VkContext* m_context;
};