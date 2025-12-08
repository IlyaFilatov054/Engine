#include "render/VkRenderer.h"
#include <GLFW/glfw3.h>
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "render/VkUtils.h"

VkRenderer::VkRenderer(const Window* window){
    m_window = window;
    createInstance();
    createSurface();
}

VkRenderer::~VkRenderer(){
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyInstance(m_instance, nullptr);
}

void VkRenderer::createInstance(){
    VkApplicationInfo appInfo {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Engine",
        .applicationVersion = VK_MAKE_VERSION(0, 1, 0),
        .pEngineName = "Engine",
        .engineVersion = VK_MAKE_VERSION(0, 1, 0),
        .apiVersion = VK_API_VERSION_1_4
    };

    std::vector<char const *> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    bool enableValidation = true;

    std::vector<char const *> requiredLayers;
    if(enableValidation) requiredLayers.assign(validationLayers.begin(), validationLayers.end());

    uint32_t extensionCount = 0;
    auto extensions = m_window->getExtensions(extensionCount);

    VkInstanceCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
        .ppEnabledLayerNames = requiredLayers.data(),
        .enabledExtensionCount = extensionCount,
        .ppEnabledExtensionNames = extensions,
    };

    auto res = vkCreateInstance(&createInfo, nullptr, &m_instance);
    validateVkResult(res, "VkInstance");
}

void VkRenderer::createSurface(){
    auto res = glfwCreateWindowSurface(m_instance, m_window->getWindow(), nullptr, &m_surface);
    validateVkResult(res, "VkSurface");
}