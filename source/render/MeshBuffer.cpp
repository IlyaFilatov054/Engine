#include "render/MeshBuffer.h"
#include <cstdint>
#include <vulkan/vulkan_core.h>
#include "render/Vertex.h"

MeshBuffer::MeshBuffer(const VkContext* context, const uint32_t vertexCount, const uint32_t indices) : 
m_vertexBuffer(context, sizeof(Vertex) * vertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
m_indexBuffer(context, sizeof(uint32_t) * indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {

}

const VkBuffer& MeshBuffer::vertexBuffer() const {
    return m_vertexBuffer.buffer();
}

void MeshBuffer::setVertexData(void* data) {
    m_vertexBuffer.stagingBuffer().setData(data);
    m_vertexBuffer.flush();
}

const VkBuffer& MeshBuffer::indexBuffer() const {
    return m_indexBuffer.buffer();
}

void MeshBuffer::setIndexData(void* data) {
    m_indexBuffer.stagingBuffer().setData(data);
    m_indexBuffer.flush();
}
const uint32_t MeshBuffer::indexBufferSize() const {
    return m_indexBuffer.size();
}

const uint32_t MeshBuffer::indicesCount() const {
    return indexBufferSize() / sizeof(uint32_t);
}