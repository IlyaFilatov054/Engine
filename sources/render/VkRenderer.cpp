#include "render/VkRenderer.h"
#include "render/RenderCore.h"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

VkRenderer::VkRenderer(const Window* window){
    m_window = window;
    m_context = new VkContext(window);
    m_swapchain = new Swapchain(m_context);
    m_core = new RenderCore(m_context, m_swapchain);
}

VkRenderer::~VkRenderer(){
    delete m_core;
    delete m_swapchain;
    delete m_context;
}

void VkRenderer::render() const {
    m_core->drawFrame();
}