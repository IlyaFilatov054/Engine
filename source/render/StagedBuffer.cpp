#include "render/StagedBuffer.h"
#include "render/AbstractBuffer.h"
#include <vulkan/vulkan_core.h>
#include "render/MappedBuffer.h"
#include "render/VkUtils.h"

StagedBuffer::StagedBuffer(const VkContext* context, uint32_t size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryFlags) : 
AbstractBuffer(
    context, size
), m_buffer(context, size, usageFlags | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    memoryFlags) {

}

const VkBuffer& StagedBuffer::buffer() const {
    return  m_buffer.buffer();
}

void StagedBuffer::setData(void* data) {
    MappedBuffer stagingBuffer(m_context, m_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stagingBuffer.setData(data);
    executeOnGpu(m_context, [&](const VkCommandBuffer commandBuffer) {
        VkBufferCopy copyRegion {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = m_size
        };
        vkCmdCopyBuffer(commandBuffer, stagingBuffer.buffer(), m_buffer.buffer(), 1, &copyRegion);
    });
}