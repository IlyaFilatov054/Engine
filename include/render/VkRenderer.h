#pragma once

#include "render/Window.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "render/VkContext.h"

class VkRenderer{
public:
    VkRenderer(const Window* window);
    ~VkRenderer();

private:
    VkContext* m_context = nullptr;
    const Window* m_window = nullptr;
};