#pragma once

#include <vulkan/vulkan.h>

class DescriptorManager {
public:
    DescriptorManager();
    ~DescriptorManager();
private:
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
};