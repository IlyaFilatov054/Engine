#include "render/DepthImage.h"
#include <vulkan/vulkan_core.h>

DepthImage::DepthImage(const VkContext* context, const Swapchain* swapchain) : m_context(context) {
    VkImageCreateInfo imageInfo {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_D32_SFLOAT,
        .extent {
            .width = swapchain->extent().width,
            .height = swapchain->extent().height,
            .depth = 1
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
    };
    vkCreateImage(m_context->device(), &imageInfo, nullptr, &m_image);
    
    VkMemoryRequirements memoryRequirments;
    vkGetImageMemoryRequirements(m_context->device(), m_image, &memoryRequirments);

    VkMemoryAllocateInfo allocInfo {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memoryRequirments.size,
        .memoryTypeIndex = m_context->findMemoryType(memoryRequirments.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
    };
    vkAllocateMemory(m_context->device(), &allocInfo, nullptr, &m_imageMemory);
    vkBindImageMemory(m_context->device(), m_image, m_imageMemory, 0);

    VkImageViewCreateInfo imageViewInfo {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = m_image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = VK_FORMAT_D32_SFLOAT,
        .subresourceRange {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }
    };
    vkCreateImageView(m_context->device(), &imageViewInfo, nullptr, &m_imageView);
}

DepthImage::~DepthImage() {
    vkDestroyImageView(m_context->device(), m_imageView, nullptr);
    vkDestroyImage(m_context->device(), m_image, nullptr);
    vkFreeMemory(m_context->device(), m_imageMemory, nullptr);
}

const VkImageView& DepthImage::view() const {
    return m_imageView;
}