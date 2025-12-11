#pragma once

#include "render/Window.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "render/VkContext.h"
#include "render/Swapchain.h"
#include "render/RenderCore.h"

class VkRenderer{
public:
    VkRenderer(const Window* window);
    ~VkRenderer();

private:
    VkContext* m_context = nullptr;
    Swapchain* m_swapchain = nullptr;
    RenderCore* m_core = nullptr;
    const Window* m_window = nullptr;
};