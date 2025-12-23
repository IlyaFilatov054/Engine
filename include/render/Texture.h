#pragma once
#include "render/AbstractBuffer.h"
#include "render/VkContext.h"
#include <cstdint>

class Texture {
public:
    Texture(const VkContext* context, const char* path);
    ~Texture();
private:
    const VkContext* m_context;

    VkImage m_image = VK_NULL_HANDLE;
    VkDeviceMemory m_memory = VK_NULL_HANDLE;
    VkImageView m_imageView = VK_NULL_HANDLE;
    VkSampler m_sampler = VK_NULL_HANDLE;

    uint32_t m_width, m_height;

    void transitionImageLayout(const VkImage image, const VkFormat format, const VkImageLayout oldLayout, const VkImageLayout newLayout) const;
    void copyBufferToImage(const AbstractBuffer* buffer);
};