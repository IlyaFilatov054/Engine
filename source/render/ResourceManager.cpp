#include "render/ResourceManager.h"
#include "render/MappedBuffer.h"
#include "render/MeshBuffer.h"
#include "render/Texture.h"
#include <cstdint>
#include <vulkan/vulkan_core.h>

ResourceManager::ResourceManager(const VkContext* context, const VkDescriptorSet ssboDescriptor, const VkDescriptorSet textureDescriptor)
 : m_context(context),
 m_ssbo(m_context, 1024, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 
    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT),
    m_ssboDescriptor(ssboDescriptor),
    m_textureDescriptor(textureDescriptor) {
    VkDescriptorBufferInfo bufferInfo {
        .buffer = m_ssbo.buffer(),
        .offset = 0,
        .range = 1024,
    };

    VkWriteDescriptorSet write {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = m_ssboDescriptor,
        .dstBinding = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .pBufferInfo = &bufferInfo,
    };
    vkUpdateDescriptorSets(m_context->device(), 1, &write, 0, nullptr);

    addTexture("textures/Missing_Texture.png");
    for(uint32_t i = 0; i < 16; i++){
        VkDescriptorImageInfo info {
            .sampler = texture(0)->sampler(),
            .imageView = texture(0)->image()->view(),
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
        VkWriteDescriptorSet write {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = m_textureDescriptor,
            .dstBinding = 0,
            .dstArrayElement = i,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &info
        };
        vkUpdateDescriptorSets(m_context->device(), 1, &write, 0, nullptr);
    }
}

ResourceManager::~ResourceManager() {
    for(auto& t : m_textures){
        delete t;
    }
    for(auto& m : m_meshes){
        delete m;
    }
}

uint32_t ResourceManager::addTexture(const char* path) {
    Texture* texture = new Texture(m_context, path);
    VkDescriptorImageInfo info {
        .sampler = texture->sampler(),
        .imageView = texture->image()->view(),
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };
    VkWriteDescriptorSet write {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = m_textureDescriptor,
        .dstBinding = 0,
        .dstArrayElement = static_cast<uint32_t>(m_textures.size()),
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &info
    };
    vkUpdateDescriptorSets(m_context->device(), 1, &write, 0, nullptr);
    m_textures.push_back(texture);
    return m_textures.size() - 1;
}

uint32_t ResourceManager::addMesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
    MeshBuffer* meshBuffer = new MeshBuffer(m_context, vertices.size(), indices.size());
    meshBuffer->setVertexData(vertices.data());
    meshBuffer->setIndexData(indices.data());
    m_meshes.push_back(meshBuffer);
    return m_meshes.size() - 1;
}

uint32_t ResourceManager::addRenderData() {
    m_renderObjectsData.emplace_back();
    return m_renderObjectsData.size() - 1;
}

void ResourceManager::updateSsbo() {
    m_ssbo.setData(m_renderObjectsData.data());
}

Texture* ResourceManager::texture(uint32_t index) const {
    return m_textures[index];
}

MeshBuffer* ResourceManager::mesh(uint32_t index) const {
    return m_meshes[index];
}

RenderObjectData& ResourceManager::renderData(uint32_t index) {
    return m_renderObjectsData[index];
}