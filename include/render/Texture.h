#pragma once
#include "render/Image.h"
#include "render/VkContext.h"
#include <cstdint>

class Texture {
public:
    Texture(const VkContext* context, const char* path);
    ~Texture();

    const Image* image() const;
    const VkSampler& sampler() const;
private:
    const VkContext* m_context;

    Image* m_image = nullptr;
    VkSampler m_sampler = VK_NULL_HANDLE;

    uint32_t m_width, m_height;
};