#pragma once

#include "render/DepthImage.h"
#include "render/Swapchain.h"
#include "render/VkContext.h"
#include <vulkan/vulkan_core.h>

class ImageResources {
public:
    ImageResources(const VkContext* context, const Swapchain* swapchain, const VkImageView imageView, const VkRenderPass renderPass);
    ~ImageResources();

    const VkFramebuffer& framebuffer() const;
    const VkSemaphore& renderFinishedSemaphore() const;

private:
    const VkContext* m_context;
    const Swapchain* m_swapchain;
    const VkImageView m_imageView;
    const VkRenderPass m_renderPass;

    VkFramebuffer m_framebuffer = VK_NULL_HANDLE;
    DepthImage m_depthImage;
    VkSemaphore m_renderFinished = VK_NULL_HANDLE;

    void createFramebuffer();
    void createSemaphore();
};