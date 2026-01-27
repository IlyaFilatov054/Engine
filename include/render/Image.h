#pragma once

#include "render/VkContext.h"
#include <map>
#include <vulkan/vulkan_core.h>
#include "render/AbstractBuffer.h"

const std::map<VkImageLayout, VkPipelineStageFlags> PIPELINE_STAGE_TABLE {
    {VK_IMAGE_LAYOUT_UNDEFINED, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT},
    {VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
    {VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT},
    {VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT},
    {VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT},
    {VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT},
};

const std::map<VkImageLayout, VkAccessFlags> ACCESS_MASK_TABLE {
    {VK_IMAGE_LAYOUT_UNDEFINED, 0},
    {VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT},
    {VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT},
    {VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT},
    {VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_READ_BIT},
    {VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT},
};

class Image {
public:
    Image(const VkContext* context, const VkFormat format);
    Image(const VkContext* context, const VkFormat format, VkImage image);
    ~Image();

    const VkImageView& view() const;
    const VkImage& image() const;
    
    void createImage( const VkImageUsageFlags usage, const VkExtent3D extent);
    void createView(const VkImageAspectFlags aspect);
    
    void transitionLayout(
        const VkImageLayout newLayout
    );
    void transitionLayout(
        const VkImageLayout newLayout,
        const VkCommandBuffer commandBuffer
    );
    void copyBufferToImage(const AbstractBuffer* buffer);

    void forceLayout(VkImageLayout layout);
    VkImageLayout layout() const;
private:
    const VkContext* m_context;
    const VkFormat m_format;

    VkImage m_image = VK_NULL_HANDLE;
    VkDeviceMemory m_memory = VK_NULL_HANDLE;
    VkImageView m_view = VK_NULL_HANDLE;
    bool m_externalImage = false;
    VkExtent3D m_extent;

    VkImageLayout m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageAspectFlags m_aspect = VK_IMAGE_ASPECT_COLOR_BIT;
};