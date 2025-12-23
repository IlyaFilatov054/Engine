#pragma once
#include "render/AbstractBuffer.h"
#include "render/Buffer.h"
#include <cstdint>
#include <vulkan/vulkan_core.h>

class StagedBuffer : public AbstractBuffer {
public:
    StagedBuffer(const VkContext* context, uint32_t size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryFlags);
    ~StagedBuffer() override = default;

    const VkBuffer& buffer() const override;
    void setData(void* data) override;
    
private:
    Buffer m_buffer;
};