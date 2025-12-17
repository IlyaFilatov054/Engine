#pragma once

#include <cstdint>
#include <glm/ext/scalar_uint_sized.hpp>
#include <vulkan/vulkan_core.h>
#include "render/VkContext.h"

class AbstractBuffer {
public:
    AbstractBuffer(const VkContext* context, const uint32_t size) : m_context(context), m_size(size) {}
    virtual ~AbstractBuffer() = default;

    const uint32_t size() const {return m_size;};
    virtual const VkBuffer& buffer() const = 0;
    virtual void setData(void* data) = 0;

protected:
    const VkContext* m_context = nullptr;
    const uint32_t m_size;
};