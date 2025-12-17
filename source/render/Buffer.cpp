#include "render/Buffer.h"
#include "render/AbstractBuffer.h"
#include "render/VkUtils.h"
#include <cstdint>
#include <vulkan/vulkan_core.h>


Buffer::Buffer(const VkContext* context, uint32_t size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryFlags) 
: AbstractBuffer(context, size) {
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

void Buffer::createBuffer(VkBufferUsageFlags usageFlags) {
    VkBufferCreateInfo bufferInfo {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = m_size,
        .usage = usageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    auto res = vkCreateBuffer(m_context->device(), &bufferInfo, nullptr, &m_buffer);
    validateVkResult(res, "vkCreateBuffer");
}

void Buffer::allocateMemory(VkMemoryPropertyFlags memoryFlags) {
    VkMemoryRequirements memoryRequirments;
    vkGetBufferMemoryRequirements(m_context->device(), m_buffer, &memoryRequirments);

    VkMemoryAllocateInfo allocateInfo {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memoryRequirments.size,
        .memoryTypeIndex = m_context->findMemoryType(memoryRequirments.memoryTypeBits, memoryFlags),
    };
    auto res = vkAllocateMemory(m_context->device(), &allocateInfo, nullptr, &m_memory);
    validateVkResult(res, "vkAllocateMemory");
    res = vkBindBufferMemory(m_context->device(), m_buffer, m_memory, 0);
    validateVkResult(res, "vkBindBufferMemory");
}