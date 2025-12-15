#pragma once
#include <cstdint>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "render/Vertex.h"
#include "render/VkContext.h"

class Buffer {
public:
    Buffer(uint32_t size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryFlags, const VkContext* context);
    ~Buffer();

    const VkBuffer& buffer() const;
    const uint32_t size() const;
    void setData(void* data);
private:
    VkBuffer m_buffer;
    VkDeviceMemory m_memory;
    void* m_mappedMemory;
    const VkContext* m_context = nullptr;
    const uint32_t m_size;

    void createBuffer(VkBufferUsageFlags usageFlags);
    void allocateMemory(VkMemoryPropertyFlags memoryFlags);
};