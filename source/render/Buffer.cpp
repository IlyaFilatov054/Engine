#include "render/Buffer.h"
#include "render/Vertex.h"
#include <cstdint>
#include <cstring>
#include <vulkan/vulkan_core.h>


Buffer::Buffer(uint32_t size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryFlags, const VkContext* context) 
: m_size(size), m_context(context) {
    createBuffer(usageFlags);
    allocateMemory(memoryFlags);
}

Buffer::~Buffer() {
    vkDestroyBuffer(m_context->device(), m_buffer, nullptr);
    vkFreeMemory(m_context->device(), m_memory, nullptr);
}

const VkBuffer& Buffer::buffer() const {
    return m_buffer;
}

const uint32_t Buffer::size() const {
    return  m_size;
}

void Buffer::setData(void* data) {
    memcpy(m_mappedMemory, data, m_size);
}

void Buffer::createBuffer(VkBufferUsageFlags usageFlags) {
    VkBufferCreateInfo bufferInfo {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = m_size,
        .usage = usageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    vkCreateBuffer(m_context->device(), &bufferInfo, nullptr, &m_buffer);
}

void Buffer::allocateMemory(VkMemoryPropertyFlags memoryFlags) {
    VkMemoryRequirements memoryRequirments;
    vkGetBufferMemoryRequirements(m_context->device(), m_buffer, &memoryRequirments);

    VkMemoryAllocateInfo allocateInfo {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memoryRequirments.size,
        .memoryTypeIndex = m_context->findMemoryType(memoryRequirments.memoryTypeBits, memoryFlags),
    };
    vkAllocateMemory(m_context->device(), &allocateInfo, nullptr, &m_memory);
    vkBindBufferMemory(m_context->device(), m_buffer, m_memory, 0);

    vkMapMemory(m_context->device(), m_memory, 0, m_size, 0, &m_mappedMemory);
}