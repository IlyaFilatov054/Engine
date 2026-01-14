#pragma once
#include "render/Buffer.h"
#include <cstddef>
#include <cstdint>

class MappedBuffer : public Buffer {
public:
    MappedBuffer(const VkContext* context, uint32_t size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryFlags);
    virtual ~MappedBuffer() override = default;

    void setData(void* data) const;
    template<typename T>
    T* getData(uint32_t index) const {
        return reinterpret_cast<T*>((std::byte*)m_mappedMemory + index * sizeof(T));
    }
private:
    void* m_mappedMemory;
};