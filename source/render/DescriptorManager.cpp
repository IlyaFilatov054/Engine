#include "render/DescriptorManager.h"
#include <cstdint>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

DescriptorManager::DescriptorManager(const VkContext* context) : m_context(context) {
    std::vector<VkDescriptorPoolSize> poolSizes {
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 128,
        },
        {
            .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 128,
        },
        {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 128,
        },
    };
    VkDescriptorPoolCreateInfo uniformPoolInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 128,
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.data(),
    };
    vkCreateDescriptorPool(m_context->device(), &uniformPoolInfo, nullptr, &m_pool);
}

DescriptorManager::~DescriptorManager() {
    for(auto& l : m_layouts) {
        vkDestroyDescriptorSetLayout(m_context->device(), l.second, nullptr);
    }
    vkDestroyDescriptorPool(m_context->device(), m_pool, nullptr);
}

void DescriptorManager::createLayout(DescriptorSetLayoutHandle handle, const std::vector<VkDescriptorSetLayoutBinding>& bindings) {
    VkDescriptorSetLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayoutCreateInfo cameraLayoutInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data(),
    };
    vkCreateDescriptorSetLayout(m_context->device(), &cameraLayoutInfo, nullptr, &layout);
    if(m_layouts.contains(handle)) throw std::runtime_error("Current descriptor set layout handle already present!");
    m_layouts[handle] = layout;
}

VkDescriptorSetLayout DescriptorManager::layout(DescriptorSetLayoutHandle handle) const {
    if(!m_layouts.contains(handle)) throw std::runtime_error("Descriptor set layout not found!");
    return m_layouts.at(handle);
}

void DescriptorManager::allocateSets(DescriptorSetHandle descriptorHandle, DescriptorSetLayoutHandle layoutHandle, uint32_t count) {
    std::vector<VkDescriptorSet> sets;
    sets.resize(count);
    std::vector<VkDescriptorSetLayout> layouts;
    for(uint32_t i = 0; i < count; i++) layouts.push_back(m_layouts.at(layoutHandle));
    VkDescriptorSetAllocateInfo ssboSetAllocInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = m_pool,
        .descriptorSetCount = count,
        .pSetLayouts = layouts.data(),
    };
    vkAllocateDescriptorSets(m_context->device(), &ssboSetAllocInfo, sets.data());
    if(m_sets.contains(descriptorHandle)) throw std::runtime_error("Current descriptor set handle already present!");
    m_sets[descriptorHandle] = sets;
}

const std::vector<VkDescriptorSet>& DescriptorManager::sets(DescriptorSetHandle descriptorHandle) const {
    if(!m_sets.contains(descriptorHandle)) throw std::runtime_error("Descriptor set not found!");
    return m_sets.at(descriptorHandle);
}