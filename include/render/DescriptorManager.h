#pragma once

#include "render/VkContext.h"
#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <map>

typedef uint32_t DescriptorSetLayoutHandle;
typedef uint32_t DescriptorSetHandle;

struct DescriptorSet {
    DescriptorSetLayoutHandle layout;
    VkDescriptorSet descriptor;
};

class DescriptorManager {
public:
    DescriptorManager(const VkContext* context);
    ~DescriptorManager();

    void createLayout(DescriptorSetLayoutHandle handle, const std::vector<VkDescriptorSetLayoutBinding>& bindings);
    VkDescriptorSetLayout layout(DescriptorSetLayoutHandle handle) const;
    
    void allocateSets(DescriptorSetHandle descriptorHandle, DescriptorSetLayoutHandle layoutHandle, uint32_t count);
    const std::vector<DescriptorSet>& sets(DescriptorSetHandle descriptorHandle) const;

private:
    const VkContext* m_context = nullptr;

    VkDescriptorPool m_pool = VK_NULL_HANDLE;
    std::map<DescriptorSetLayoutHandle, VkDescriptorSetLayout> m_layouts;
    std::map<DescriptorSetHandle, std::vector<DescriptorSet>> m_sets;
};