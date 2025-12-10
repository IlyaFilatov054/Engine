#include "render/VkRenderer.h"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

VkRenderer::VkRenderer(const Window* window){
    m_window = window;
    m_context = new VkContext(window);
    m_swapchain = new Swapchain(m_context);
}

VkRenderer::~VkRenderer(){
    delete m_swapchain;
    delete m_context;
}