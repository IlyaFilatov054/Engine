#pragma once
#include "render/Buffer.h"

class MappedBuffer : public Buffer {
public:
    MappedBuffer(const VkContext* context, uint32_t size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryFlags);
    virtual ~MappedBuffer() override = default;

    void setData(void* data) override;

private:
    void* m_mappedMemory;
};