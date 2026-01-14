#include "render/DescriptorManager.h"
#include <vulkan/vulkan_core.h>

DescriptorManager::DescriptorManager(const VkContext* context) : m_context(context) {
    createLayouts();
    createPools();
    allocateCameraDescriptor();
    allocateTexturesDescriptor();
}

DescriptorManager::~DescriptorManager() {
    vkDestroyDescriptorSetLayout(m_context->device(), m_cameraLayout, nullptr);
    vkDestroyDescriptorSetLayout(m_context->device(), m_ssboLayout, nullptr);
    vkDestroyDescriptorSetLayout(m_context->device(), m_texturesLayout, nullptr);

    vkDestroyDescriptorPool(m_context->device(), m_uniformPool, nullptr);
    vkDestroyDescriptorPool(m_context->device(), m_storagePool, nullptr);
    vkDestroyDescriptorPool(m_context->device(), m_samplerPool, nullptr);
}

const VkDescriptorSet& DescriptorManager::cameraSet() const {
    return m_cameraSet;
}

const VkDescriptorSetLayout& DescriptorManager::cameraLayout() const {
    return m_cameraLayout;
}

const VkDescriptorSetLayout& DescriptorManager::ssboLayout() const {
    return m_ssboLayout;
}

const VkDescriptorSet& DescriptorManager::texturesSet() const {
    return m_texturesSet;
}

const VkDescriptorSetLayout& DescriptorManager::texturesLayout() const {
    return m_texturesLayout;
}

void DescriptorManager::createLayouts() {
    VkDescriptorSetLayoutBinding cameraBinding {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT
    };
    VkDescriptorSetLayoutCreateInfo cameraLayoutInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &cameraBinding
    };
    vkCreateDescriptorSetLayout(m_context->device(), &cameraLayoutInfo, nullptr, &m_cameraLayout);   

    VkDescriptorSetLayoutBinding ssboBinding {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT
    };
    VkDescriptorSetLayoutCreateInfo ssboLayoutInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &ssboBinding
    };
    vkCreateDescriptorSetLayout(m_context->device(), &ssboLayoutInfo, nullptr, &m_ssboLayout);

    VkDescriptorSetLayoutBinding texturesBinding {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 16,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };
    VkDescriptorSetLayoutCreateInfo texturesLayoutInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &texturesBinding
    };
    vkCreateDescriptorSetLayout(m_context->device(), &texturesLayoutInfo, nullptr, &m_texturesLayout);
}

void DescriptorManager::createPools() {
    VkDescriptorPoolSize uniformPoolSize {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
    };
    VkDescriptorPoolCreateInfo uniformPoolInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 1,
        .poolSizeCount = 1,
        .pPoolSizes = &uniformPoolSize,
    };
    vkCreateDescriptorPool(m_context->device(), &uniformPoolInfo, nullptr, &m_uniformPool);

    VkDescriptorPoolSize storagePoolSize {
        .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 8,
    };
    VkDescriptorPoolCreateInfo storagePoolInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 8,
        .poolSizeCount = 1,
        .pPoolSizes = &storagePoolSize,
    };
    vkCreateDescriptorPool(m_context->device(), &storagePoolInfo, nullptr, &m_storagePool);

    VkDescriptorPoolSize samplerPoolSize {
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 16,
    };
    VkDescriptorPoolCreateInfo samplerPoolInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 1,
        .poolSizeCount = 1,
        .pPoolSizes = &samplerPoolSize,
    };
    vkCreateDescriptorPool(m_context->device(), &samplerPoolInfo, nullptr, &m_samplerPool);
}

VkDescriptorSet DescriptorManager::allocateStorageDescriptor() const {
    VkDescriptorSet ssboSet = VK_NULL_HANDLE;
    VkDescriptorSetAllocateInfo ssboSetAllocInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = m_storagePool,
        .descriptorSetCount = 1,
        .pSetLayouts = &m_ssboLayout
    };
    vkAllocateDescriptorSets(m_context->device(), &ssboSetAllocInfo, &ssboSet);
    return ssboSet;
}

void DescriptorManager::allocateCameraDescriptor() {
    VkDescriptorSetAllocateInfo cameraSetAllocInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = m_uniformPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &m_cameraLayout
    };
    vkAllocateDescriptorSets(m_context->device(), &cameraSetAllocInfo, &m_cameraSet);
}

void DescriptorManager::allocateTexturesDescriptor() {
    VkDescriptorSetAllocateInfo texturesSetAllocInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = m_samplerPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &m_texturesLayout
    };
    vkAllocateDescriptorSets(m_context->device(), &texturesSetAllocInfo, &m_texturesSet);
}