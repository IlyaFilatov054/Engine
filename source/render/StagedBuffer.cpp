#include "render/StagedBuffer.h"
#include "render/AbstractBuffer.h"
#include <vulkan/vulkan_core.h>
#include "render/MappedBuffer.h"

StagedBuffer::StagedBuffer(const VkContext* context, uint32_t size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryFlags, const VkCommandPool pool) : 
AbstractBuffer(
    context, size
), m_buffer(context, size, usageFlags | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    memoryFlags
), m_pool(pool) {

}

const VkBuffer& StagedBuffer::buffer() const {
    return  m_buffer.buffer();
}

void StagedBuffer::setData(void* data) {
    MappedBuffer stagingBuffer(m_context, m_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stagingBuffer.setData(data);

    VkCommandBufferAllocateInfo allocInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };
    VkCommandBuffer copyBuffer= VK_NULL_HANDLE;
    vkAllocateCommandBuffers(m_context->device(), &allocInfo, &copyBuffer);

    VkCommandBufferBeginInfo beginInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    vkBeginCommandBuffer(copyBuffer, &beginInfo);

    VkBufferCopy copyRegion {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = m_size
    };
    vkCmdCopyBuffer(copyBuffer, stagingBuffer.buffer(), m_buffer.buffer(), 1, &copyRegion);

    vkEndCommandBuffer(copyBuffer);

    VkSubmitInfo submitInfo {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &copyBuffer,
    };
    vkQueueSubmit(m_context->graphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_context->graphicsQueue());

    vkFreeCommandBuffers(m_context->device(), m_pool, 1, &copyBuffer);
}