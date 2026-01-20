#include "render/Texture.h"
#include "render/Image.h"
#include <cstdint>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <vulkan/vulkan_core.h>
#include "render/MappedBuffer.h"

Texture::Texture(const VkContext* context, const char* path) : m_context(context) {
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    auto pixels = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);

    m_width = width;
    m_height = height;

    VkDeviceSize imageSize = m_width * m_height * 4;

    MappedBuffer stagingBuffer(context, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    stagingBuffer.setData(pixels);
    stbi_image_free(pixels);

    m_image = new Image(m_context, VK_FORMAT_R8G8B8A8_SRGB);
    m_image->createImage(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, {(uint32_t)width, (uint32_t)height, 1});
    m_image->transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);
    m_image->copyBufferToImage(&stagingBuffer);
    m_image->transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT);
    m_image->createView(VK_IMAGE_ASPECT_COLOR_BIT);

    VkSamplerCreateInfo samplerInfo {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .anisotropyEnable = VK_FALSE,
        .maxAnisotropy = 0,
        .compareEnable = VK_FALSE,
        .unnormalizedCoordinates = VK_FALSE,
    };
    vkCreateSampler(m_context->device(), &samplerInfo, nullptr, &m_sampler);
}

Texture::~Texture() {
    vkDestroySampler(m_context->device(), m_sampler, nullptr);
    delete m_image;
}

const Image* Texture::image() const {
    return m_image;
}

const VkSampler& Texture::sampler() const {
    return m_sampler;
}