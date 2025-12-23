#pragma once

#include "render/VkContext.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

class DescriptorManager {
public:
    DescriptorManager(const VkContext* context);
    ~DescriptorManager();

    const VkDescriptorSet& cameraDescriptorSet() const;
    const VkDescriptorSetLayout& layout() const;
private:
    const VkContext* m_context = nullptr;
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet m_cameraDescriptorSet = VK_NULL_HANDLE;

    void createLayouts();
    void createPool();
    void allocateDescriptors();
};