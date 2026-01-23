#include "render/Image.h"
#include "render/VkUtils.h"
#include <vulkan/vulkan_core.h>

Image::Image(const VkContext* context, const VkFormat format)
 : m_context(context), m_format(format) {
}

Image::Image(const VkContext* context, const VkFormat format, VkImage image)
 : m_context(context), m_format(format) {
    m_externalImage = true;
    m_image = image;
}

Image::~Image() {
    vkDestroyImageView(m_context->device(), m_view, nullptr);
    if(m_externalImage) return;
    vkDestroyImage(m_context->device(), m_image, nullptr);
    vkFreeMemory(m_context->device(), m_memory, nullptr);
}

const VkImageView& Image::view() const {
    return m_view;
}

const VkImage& Image::image() const {
    return m_image;
}

void Image::createImage(const VkImageUsageFlags usage, const VkExtent3D extent) {
    m_extent = extent;
    VkImageCreateInfo imageInfo {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = m_format,
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

void Image::createView(const VkImageAspectFlags aspect) {
    m_aspect = aspect;
    VkImageViewCreateInfo imageViewInfo {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = m_image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = m_format,
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

void Image::transitionLayout(
    const VkImageLayout layout,
    const VkPipelineStageFlags stage,
    const VkAccessFlags accessMask
) const {
    executeOnGpu(m_context, [&](const VkCommandBuffer commandBuffer) {
        transitionLayout(layout, stage, accessMask, commandBuffer);
    });
}

void Image::transitionLayout(
        const VkImageLayout layout,
        const VkPipelineStageFlags stage,
        const VkAccessFlags accessMask,
        const VkCommandBuffer commandBuffer
) const {
    VkImageMemoryBarrier barrier {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = m_accessMask,
        .dstAccessMask = accessMask,

        .oldLayout = m_layout,
        .newLayout = layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = m_image,
        .subresourceRange {
            .aspectMask = m_aspect,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
    };
    vkCmdPipelineBarrier(commandBuffer, m_stage, stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void Image::copyBufferToImage(const AbstractBuffer* buffer) {
    VkBufferImageCopy region {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource {
            .aspectMask = m_aspect,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
        .imageOffset = {0,0,0},
        .imageExtent = {
            .width = m_extent.width,
            .height = m_extent.height,
            .depth = 1
        }
    };
    executeOnGpu(m_context, [&](const VkCommandBuffer commandBuffer) {
        vkCmdCopyBufferToImage(commandBuffer, buffer->buffer(), m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    });
}

void Image::setLayout(VkImageLayout layout) {
    m_layout = layout;
}

VkImageLayout Image::layout() const {
    return m_layout;
}

void Image::setPipelineStage(VkPipelineStageFlags stage) {
    m_stage = stage;
}

VkPipelineStageFlags Image::pipelineStage() const {
    return m_stage;
}

void Image::setAccessMask(VkAccessFlags mask) {
    m_accessMask = mask;
}

VkAccessFlags Image::accessMask() const {
    return m_accessMask;
}

void Image::setAspect(VkImageAspectFlags aspect) {
    m_aspect = aspect;
}

VkImageAspectFlags Image::aspect() const {
    return m_aspect;
}