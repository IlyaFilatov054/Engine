#pragma once

#include "render/Window.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

class VkRenderer{
public:
    VkRenderer(const Window* window);
    ~VkRenderer();

private:
    void createInstance();
    void createSurface();

    VkInstance m_instance = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    const Window* m_window = nullptr;
};