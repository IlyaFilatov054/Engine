#pragma once

#include "render/MeshBuffer.h"
#include "render/RenderObjectData.h"
#include "render/Texture.h"
#include "render/Vertex.h"
#include "render/VkContext.h"
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>

class ResourceManager {
public:
    ResourceManager(const VkContext* context, const VkDescriptorSet textureDescriptor);
    ~ResourceManager();

    uint32_t addTexture(const char* path);
    uint32_t addMesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
    uint32_t addRenderData();

    Texture* texture(uint32_t index) const;
    MeshBuffer* mesh(uint32_t index) const;
    RenderObjectData& renderData(uint32_t index);
    void* renderData();
private:
    const VkContext* m_context;
    const VkDescriptorSet m_textureDescriptor;

    std::vector<Texture*> m_textures;
    std::vector<MeshBuffer*> m_meshes;

    std::vector<RenderObjectData> m_ssboData;
};