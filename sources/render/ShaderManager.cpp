#include "render/ShaderManager.h"
#include "render/VkUtils.h"
#include <stdexcept>

ShaderManager::ShaderManager(const VkContext* context)
 : m_context(context) {
}

ShaderManager::~ShaderManager() {
    for(const auto& s : m_shaders){
        vkDestroyShaderModule(m_context->device(), s.second, nullptr);
    }
}

void ShaderManager::createShaderModule(const std::vector<char> code, const std::string& name) {
    VkShaderModuleCreateInfo shaderInfo {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = code.size(),
        .pCode = reinterpret_cast<const uint32_t*>(code.data()),
    };

    VkShaderModule shaderModule;
    auto res = vkCreateShaderModule(m_context->device(), &shaderInfo, nullptr, &shaderModule);
    validateVkResult(res, "vkCreateShaderModule");

    m_shaders[name] = shaderModule;
}

const VkShaderModule& ShaderManager::getShaderModule(const std::string& name) const {
    auto it = m_shaders.find(name);
    if(it == m_shaders.end()) throw std::runtime_error("Shader not found!");
    return it->second;
}