#pragma once

#include "render/VkContext.h"
#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

typedef uint32_t DescriptorSetLayoutHandle;

class DescriptorManager {
public:
    DescriptorManager(const VkContext* context);
    ~DescriptorManager();

    DescriptorSetLayoutHandle createLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);
    VkDescriptorSetLayout layout(DescriptorSetLayoutHandle handle) const;
    
    std::vector<VkDescriptorSet> allocateSets(DescriptorSetLayoutHandle layoutHandle, uint32_t count) const;
    VkDescriptorSet allocateSet(DescriptorSetLayoutHandle layoutHandle) const;
private:
    const VkContext* m_context = nullptr;

    VkDescriptorPool m_pool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSetLayout> m_layouts;
};