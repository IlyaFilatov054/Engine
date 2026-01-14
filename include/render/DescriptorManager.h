#pragma once

#include "render/VkContext.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

class DescriptorManager {
public:
    DescriptorManager(const VkContext* context);
    ~DescriptorManager();

    const VkDescriptorSet& cameraSet() const;
    const VkDescriptorSetLayout& cameraLayout() const;
    const VkDescriptorSetLayout& ssboLayout() const;
    const VkDescriptorSet& texturesSet() const;
    const VkDescriptorSetLayout& texturesLayout() const;
    VkDescriptorSet allocateStorageDescriptor() const;
private:
    const VkContext* m_context = nullptr;
    VkDescriptorPool m_uniformPool = VK_NULL_HANDLE;
    VkDescriptorPool m_storagePool = VK_NULL_HANDLE;
    VkDescriptorPool m_samplerPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_cameraLayout = VK_NULL_HANDLE;
    VkDescriptorSet m_cameraSet = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_ssboLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_texturesLayout = VK_NULL_HANDLE;
    VkDescriptorSet m_texturesSet = VK_NULL_HANDLE;

    void createLayouts();
    void createPools();
    void allocateCameraDescriptor();
    void allocateTexturesDescriptor();
};