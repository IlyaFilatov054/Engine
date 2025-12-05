#include "render/VkRenderer.h"
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vulkan/vulkan_core.h>

VkRenderer::VkRenderer(const Window* window){
    m_window = window;
    createInstance();
    createSurface();
}

VkRenderer::~VkRenderer(){
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyInstance(m_instance, nullptr);
}

void VkRenderer::validateResult(const VkResult& result, const char* message){
    if(result == VK_SUCCESS) return;
    std::cout << message;
    exit(1);
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

    const char* validationLayers[] = {
        "VK_LAYER_KHRONOS_validation"
    };
    bool enableValidation = true;

    uint32_t extensionCount = 0;
    auto extensions = m_window->getExtensions(extensionCount);

    VkInstanceCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = 0,
        .enabledExtensionCount = extensionCount,
        .ppEnabledExtensionNames = extensions,


    };

    if(enableValidation){
        createInfo.enabledLayerCount = 1;
        createInfo.ppEnabledLayerNames = validationLayers;
    }

    auto res = vkCreateInstance(&createInfo, nullptr, &m_instance);
    validateResult(res, "VkInstance");
}

void VkRenderer::createSurface(){
    
}