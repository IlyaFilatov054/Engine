#pragma once

#include "render/VkContext.h"
#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

class DescriptorManager {
public:
    DescriptorManager(const VkContext* context);
    ~DescriptorManager();

    uint32_t createLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);
    const VkDescriptorSetLayout& layout(uint32_t id) const;
    std::vector<VkDescriptorSet> allocateSets(uint32_t layout, uint32_t count) const;
    VkDescriptorSet allocateSet(uint32_t layout) const;
private:
    const VkContext* m_context = nullptr;

    VkDescriptorPool m_pool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSetLayout> m_layouts;
};