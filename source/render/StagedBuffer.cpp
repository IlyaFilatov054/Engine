#include "render/StagedBuffer.h"
#include "render/AbstractBuffer.h"
#include <vulkan/vulkan_core.h>
#include "render/MappedBuffer.h"
#include "render/VkUtils.h"

StagedBuffer::StagedBuffer(const VkContext* context, uint32_t size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryFlags) : 
AbstractBuffer(
    context, size
), m_buffer(
        context, 
        size, 
        usageFlags | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        memoryFlags),
    m_stagingBuffer(
        context, 
        size, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    ) {

}

VkBuffer StagedBuffer::buffer() const {
    return  m_buffer.buffer();
}

MappedBuffer& StagedBuffer::stagingBuffer() {
    return m_stagingBuffer;
}

void StagedBuffer::flushNow(uint32_t offset, uint32_t size) {
    executeOnGpu(m_context, [&](const VkCommandBuffer commandBuffer) {
        flush(offset, size, commandBuffer);
    });
}

void StagedBuffer::flushNow() {
    flushNow(0, m_size);
}

void StagedBuffer::flush(uint32_t offset, uint32_t size, const VkCommandBuffer& commandBuffer) {
    VkBufferCopy copyRegion {
            .srcOffset = offset,
            .dstOffset = offset,
            .size = size
        };
    vkCmdCopyBuffer(commandBuffer, m_stagingBuffer.buffer(), m_buffer.buffer(), 1, &copyRegion);
}

void StagedBuffer::flush(const VkCommandBuffer& commandBuffer) {
    flush(0, m_size, commandBuffer);
}