#pragma once
#include <cstdint>
#include <stdexcept>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "render/VkContext.h"
#include "render/AbstractBuffer.h"

class Buffer : public AbstractBuffer {
public:
    Buffer(const VkContext* context, uint32_t size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryFlags);
    virtual ~Buffer() override;

    const VkBuffer& buffer() const override;    
protected:
    VkBuffer m_buffer;
    VkDeviceMemory m_memory;

private:
    void createBuffer(VkBufferUsageFlags usageFlags);
    void allocateMemory(VkMemoryPropertyFlags memoryFlags);
};