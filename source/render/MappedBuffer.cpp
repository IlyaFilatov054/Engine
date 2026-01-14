#include "render/MappedBuffer.h"
#include "render/Buffer.h"
#include <cstddef>
#include <cstring>
#include <vulkan/vulkan_core.h>
#include "render/VkUtils.h"

MappedBuffer::MappedBuffer(const VkContext* context, uint32_t size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryFlags) 
: Buffer(context, size, usageFlags, memoryFlags) {
    auto res = vkMapMemory(m_context->device(), m_memory, 0, m_size, 0, &m_mappedMemory);
    validateVkResult(res, "vkMapMemory");
}

void MappedBuffer::setData(void* data) const {
    setData(data, m_size, 0);
}

void MappedBuffer::setData(void* data, uint32_t size, uint32_t offset) const {
    memcpy(reinterpret_cast<std::byte*>(m_mappedMemory) + offset, data, size);
}
