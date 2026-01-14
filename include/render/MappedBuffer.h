#pragma once
#include "render/Buffer.h"
#include <cstddef>
#include <cstdint>

class MappedBuffer : public Buffer {
public:
    MappedBuffer(const VkContext* context, uint32_t size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryFlags);
    virtual ~MappedBuffer() override = default;

    void setData(void* data) const;
    void setData(void* data, uint32_t size, uint32_t offset) const;
private:
    void* m_mappedMemory;
};