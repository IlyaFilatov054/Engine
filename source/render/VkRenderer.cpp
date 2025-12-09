#include "render/VkRenderer.h"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
#include "render/VkContext.h"

VkRenderer::VkRenderer(const Window* window){
    m_window = window;
    m_context = new VkContext(window);
}

VkRenderer::~VkRenderer(){
    delete m_context;
}