#pragma once

#include "render/VkContext.h"
#include <vulkan/vulkan_core.h>

class Image {
public:
    Image(const VkContext* context);
    Image(const VkContext* context, VkImage image);
    ~Image();

    const VkImageView& view() const;
    const VkImage& image() const;
    
    void createImage(const VkFormat format, const VkImageUsageFlags usage, const VkExtent3D extent);
    void createView(const VkFormat format, const VkImageAspectFlags aspect);
private:
    const VkContext* m_context;

    VkImage m_image = VK_NULL_HANDLE;
    VkDeviceMemory m_memory = VK_NULL_HANDLE;
    VkImageView m_view = VK_NULL_HANDLE;
    bool m_externalImage = false;
};