#include "render/DescriptorManager.h"
#include <vulkan/vulkan_core.h>

DescriptorManager::DescriptorManager(const VkContext* context) : m_context(context) {
    createLayouts();
    createPool();
    allocateDescriptors();
}

DescriptorManager::~DescriptorManager() {
    vkDestroyDescriptorPool(m_context->device(), m_descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(m_context->device(), m_descriptorSetLayout, nullptr);
}

const VkDescriptorSet& DescriptorManager::cameraDescriptorSet() const {
    return m_cameraDescriptorSet;
}

const VkDescriptorSetLayout& DescriptorManager::layout() const {
    return m_descriptorSetLayout;
}

void DescriptorManager::createLayouts() {
    VkDescriptorSetLayoutBinding cameraBinding {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT
    };
    VkDescriptorSetLayoutCreateInfo layoutInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &cameraBinding
    };
    vkCreateDescriptorSetLayout(m_context->device(), &layoutInfo, nullptr, &m_descriptorSetLayout);   
}

void DescriptorManager::createPool() {
    VkDescriptorPoolSize poolSize {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
    };
    VkDescriptorPoolCreateInfo poolInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 1,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize,
    };
    vkCreateDescriptorPool(m_context->device(), &poolInfo, nullptr, &m_descriptorPool);
}

void DescriptorManager::allocateDescriptors() {
        VkDescriptorSetAllocateInfo setAllocInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = m_descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &m_descriptorSetLayout
    };
    vkAllocateDescriptorSets(m_context->device(), &setAllocInfo, &m_cameraDescriptorSet);
}