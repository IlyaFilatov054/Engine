#pragma once
#include "render/AbstractBuffer.h"
#include "render/Buffer.h"
#include "render/MappedBuffer.h"
#include <cstdint>
#include <vulkan/vulkan_core.h>

class StagedBuffer : public AbstractBuffer {
public:
    StagedBuffer(const VkContext* context, uint32_t size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryFlags);
    ~StagedBuffer() override = default;

    const VkBuffer& buffer() const override;
    const MappedBuffer& stagingBuffer() const;
    void flushNow(uint32_t offset, uint32_t size) const;
    void flushNow() const;
    void flush(uint32_t offset, uint32_t size, const VkCommandBuffer& commandBuffer) const;
    void flush(const VkCommandBuffer& commandBuffer) const;
    
private:
    Buffer m_buffer;
    MappedBuffer m_stagingBuffer;
};