#include "render/DescriptorManager.h"
#include <cstdint>
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
        vkDestroyDescriptorSetLayout(m_context->device(), l, nullptr);
    }
    vkDestroyDescriptorPool(m_context->device(), m_pool, nullptr);
}

uint32_t DescriptorManager::createLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings) {
    VkDescriptorSetLayout layout = VK_NULL_HANDLE;
    VkDescriptorSetLayoutCreateInfo cameraLayoutInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data(),
    };
    vkCreateDescriptorSetLayout(m_context->device(), &cameraLayoutInfo, nullptr, &layout);
    m_layouts.push_back(layout);
    return m_layouts.size() - 1;
}

const VkDescriptorSetLayout& DescriptorManager::layout(uint32_t id) const {
    return m_layouts[id];
}

std::vector<VkDescriptorSet> DescriptorManager::allocateSets(uint32_t layout, uint32_t count) const {
    std::vector<VkDescriptorSet> sets(count);
    VkDescriptorSetAllocateInfo ssboSetAllocInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = m_pool,
        .descriptorSetCount = count,
        .pSetLayouts = &m_layouts[layout],
    };
    vkAllocateDescriptorSets(m_context->device(), &ssboSetAllocInfo, sets.data());
    return sets;
}

VkDescriptorSet DescriptorManager::allocateSet(uint32_t layout) const {
    return allocateSets(layout, 1)[0];
}