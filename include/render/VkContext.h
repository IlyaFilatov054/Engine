#pragma once
#include <cstdint>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "render/PhysicalDevices.h"
#include "render/Window.h"

class VkContext {
public:
    VkContext(const Window* window);
    ~VkContext();

    VkDevice device() const;
    VkPhysicalDevice physicalDevice() const;
    VkSurfaceKHR surface() const;
    VkQueue graphicsQueue() const;
    uint32_t graphicsQueueIndex() const;
    const Window* window() const;

private:
    VkInstance m_instance = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    PhysicalDevices* m_physicalDevices = nullptr;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueIndex = 0;
    const Window* m_window = nullptr;

    void createInstance();
    void createSurface();
    void initPysicalDevices();
    void createLogicalDevice();
};