#include "render/MappedBuffer.h"
#include "render/Buffer.h"
#include <cstring>
#include "render/VkUtils.h"

MappedBuffer::MappedBuffer(const VkContext* context, uint32_t size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryFlags) 
: Buffer(context, size, usageFlags, memoryFlags) {
    auto res = vkMapMemory(m_context->device(), m_memory, 0, m_size, 0, &m_mappedMemory);
    validateVkResult(res, "vkMapMemory");
}

void MappedBuffer::setData(void* data) {
    memcpy(m_mappedMemory, data, m_size);
}