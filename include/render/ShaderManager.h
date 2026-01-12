#pragma once

#include "render/VkContext.h"
#include <map>
#include <string>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

class ShaderManager {
public:
    ShaderManager(const VkContext* context);
    ~ShaderManager();
    void createShaderModule(const std::vector<char> code, const std::string& name);
    const VkShaderModule& getShaderModule(const std::string& name) const;
private:
    const VkContext* m_context;

    std::map<std::string, VkShaderModule> m_shaders;
};