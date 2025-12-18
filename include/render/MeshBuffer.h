#pragma once

#include "render/StagedBuffer.h"
#include "render/VkContext.h"
#include <cstdint>
#include <vulkan/vulkan_core.h>

class MeshBuffer {
public:
    MeshBuffer(const VkContext* context, const uint32_t size, const uint32_t indices, const VkCommandPool pool);
    ~MeshBuffer() = default;

    const VkBuffer& vertexBuffer() const;
    void setVertexData(void* data);
    const uint32_t vertexBufferSize() const;

    const VkBuffer& indexBuffer() const;
    void setIndexData(void* data);
    const uint32_t indexBufferSize() const;
    const uint32_t indicesCount() const;
private:
    StagedBuffer m_vertexBuffer;
    StagedBuffer m_indexBuffer;
};