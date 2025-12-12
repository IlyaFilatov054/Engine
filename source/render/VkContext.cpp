#include "render/VkContext.h"
#include "render/VkUtils.h"
#include "render/Window.h"
#include <iostream>
#include <vulkan/vulkan_core.h>

VkContext::VkContext(const Window* window){
    m_window = window;
    createInstance();
    createSurface();
    initPysicalDevices();
    createLogicalDevice();
}

VkContext::~VkContext(){
    vkDestroyDevice(m_device, nullptr);
    delete m_physicalDevices;
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyInstance(m_instance, nullptr);
}

void VkContext::createInstance(){
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

void VkContext::createSurface(){
    auto res = glfwCreateWindowSurface(m_instance, m_window->getWindow(), nullptr, &m_surface);
    validateVkResult(res, "VkSurface");
}

void VkContext::initPysicalDevices() {
    m_physicalDevices = new PhysicalDevices(m_instance, m_surface);
    auto devices = m_physicalDevices->getDevices();
    std::cout << devices.size() << '\n';
    for(size_t i = 0; i < devices.size(); i++) {
        if(!devices[i]->hasExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME)) continue;
        auto queues = devices[i]->getQueues(true, VK_QUEUE_GRAPHICS_BIT);
        if(queues.size() < 1) continue;
        m_physicalDevices->selectDevice(i);
        return;
    }
    vkRendererError("No suitable device!");   
}

void VkContext::createLogicalDevice() {
    auto queues = m_physicalDevices->selected()->getQueues(true, VK_QUEUE_GRAPHICS_BIT);
    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    
    float priority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = queues[0]->index(),
        .queueCount = 1,
        .pQueuePriorities = &priority,
    };

    VkDeviceCreateInfo createinfo {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCreateInfo,
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
    };
    auto res = vkCreateDevice(m_physicalDevices->selected()->device(), &createinfo, nullptr, &m_device);
    validateVkResult(res, "vkCreateDevice");

    vkGetDeviceQueue(m_device, queues[0]->index(), 0, &m_graphicsQueue);
    m_graphicsQueueIndex = queues[0]->index();
}

VkDevice VkContext::device() const {
    return m_device;
}

VkPhysicalDevice VkContext::physicalDevice() const {
    return m_physicalDevices->selected()->device();
}

VkSurfaceKHR VkContext::surface() const {
    return m_surface;
}

VkQueue VkContext::graphicsQueue() const {
    return m_graphicsQueue;
}

uint32_t VkContext::graphicsQueueIndex() const {
    return m_graphicsQueueIndex;
}

const Window* VkContext::window() const {
    return m_window;
}