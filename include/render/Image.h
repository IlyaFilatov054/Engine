#pragma once

#include "render/VkContext.h"
#include <vulkan/vulkan_core.h>

enum ImageType {
    Color = 0,
    Depth = 1,
};

class Image {
public:
    Image(const VkContext* context, const VkFormat format, const ImageType type, const VkExtent3D extent);
    Image(const VkContext* context, const VkFormat format, const ImageType type, VkImage image);
    ~Image();

    const VkImageView& view() const;
private:
    const VkContext* m_context;

    VkImage m_image = VK_NULL_HANDLE;
    VkDeviceMemory m_memory = VK_NULL_HANDLE;
    VkImageView m_view = VK_NULL_HANDLE;

    void createImage(VkFormat format, VkExtent3D extent, VkImageUsageFlags usage);
    void createView(VkFormat format, VkImageAspectFlags aspect);
};