#pragma once

#include "render/VkContext.h"
#include <vulkan/vulkan_core.h>
#include "render/AbstractBuffer.h"

class Image {
public:
    Image(const VkContext* context, const VkFormat format);
    Image(const VkContext* context, const VkFormat format, VkImage image);
    ~Image();

    const VkImageView& view() const;
    const VkImage& image() const;
    
    void createImage( const VkImageUsageFlags usage, const VkExtent3D extent);
    void createView(const VkImageAspectFlags aspect);
    
    void transitionLayout(const VkImageLayout oldLayout, const VkImageLayout newLayout) const;
    void copyBufferToImage(const AbstractBuffer* buffer);
private:
    const VkContext* m_context;
    const VkFormat m_format;

    VkImage m_image = VK_NULL_HANDLE;
    VkDeviceMemory m_memory = VK_NULL_HANDLE;
    VkImageView m_view = VK_NULL_HANDLE;
    bool m_externalImage = false;
    VkExtent3D m_extent;
    
};