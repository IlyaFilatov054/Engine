#include "render/Image.h"
#include "render/VkUtils.h"
#include <map>
#include <vulkan/vulkan_core.h>

const std::map<ImageType, VkImageUsageFlags> imageUsageFlags {
    {ImageType::Color, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT},
    {ImageType::Depth, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT},
};

const std::map<ImageType, VkImageAspectFlags> imageAspectFlags {
    {ImageType::Color, VK_IMAGE_ASPECT_COLOR_BIT},
    {ImageType::Depth, VK_IMAGE_ASPECT_DEPTH_BIT},
};

Image::Image(const VkContext* context, const VkFormat format, const ImageType type, const VkExtent3D extent)
 : m_context(context) {
    createImage(format, extent, imageUsageFlags.at(type));
    createView(format, imageAspectFlags.at(type));
}

Image::Image(const VkContext* context, const VkFormat format, const ImageType type, VkImage image)
 : m_context(context) {
    m_image = image;
    createView(format, imageAspectFlags.at(type));
}

Image::~Image() {
    vkDestroyImageView(m_context->device(), m_view, nullptr);
    if(m_memory == VK_NULL_HANDLE) return;
    vkDestroyImage(m_context->device(), m_image, nullptr);
    vkFreeMemory(m_context->device(), m_memory, nullptr);
}

const VkImageView& Image::view() const {
    return m_view;
}

void Image::createImage(VkFormat format, VkExtent3D extent, VkImageUsageFlags usage) {
    VkImageCreateInfo imageInfo {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = extent,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = usage,
    };
    auto res = vkCreateImage(m_context->device(), &imageInfo, nullptr, &m_image);
    validateVkResult(res, "vkCreateImage");

    VkMemoryRequirements memoryRequirments;
    vkGetImageMemoryRequirements(m_context->device(), m_image, &memoryRequirments);

    VkMemoryAllocateInfo allocInfo {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memoryRequirments.size,
        .memoryTypeIndex = m_context->findMemoryType(memoryRequirments.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
    };
    res = vkAllocateMemory(m_context->device(), &allocInfo, nullptr, &m_memory);
    validateVkResult(res, "vkAllocateMemory");

    res = vkBindImageMemory(m_context->device(), m_image, m_memory, 0);
    validateVkResult(res, "vkBindImageMemory");
}

void Image::createView(VkFormat format, VkImageAspectFlags aspect) {
    VkImageViewCreateInfo imageViewInfo {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = m_image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
        .subresourceRange {
            .aspectMask = aspect,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }
    };
    auto res = vkCreateImageView(m_context->device(), &imageViewInfo, nullptr, &m_view);
    validateVkResult(res, "vkCreateImageView");
}