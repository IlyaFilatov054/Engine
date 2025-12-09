#include "render/Swapchain.h"
#include <algorithm>
#include <cstdint>
#include <vulkan/vulkan_core.h>
#include "render/VkUtils.h"

Swapchain::Swapchain(const VkPhysicalDevice device, const VkSurfaceKHR surface, const Window* window) {
    auto res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &m_capabilities);
    validateVkResult(res, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");

    uint32_t formatCount;
    res = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    validateVkResult(res, "vkGetPhysicalDeviceSurfaceFormatsKHR 1");
    m_formats.resize(formatCount);
    res = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, m_formats.data());
    validateVkResult(res, "vkGetPhysicalDeviceSurfaceFormatsKHR 2");
    m_selectedFormat = m_formats[0];
    for(const auto& f : m_formats){
        if(f.format == VK_FORMAT_B8G8R8A8_SRGB && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
            m_selectedFormat = f;
            break;
        }
    }

    uint32_t presentModeCount;
    res = vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    validateVkResult(res, "vkGetPhysicalDeviceSurfacePresentModesKHR 1");
    m_presentModes.resize(presentModeCount);
    res = vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, m_presentModes.data());
    validateVkResult(res, "vkGetPhysicalDeviceSurfacePresentModesKHR 2");
    m_selectedPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    for(const auto& m : m_presentModes){
        if(m == VK_PRESENT_MODE_MAILBOX_KHR){
            m_selectedPresentMode = m;
            break;
        }
    }

    int w, h;
    window->getSize(w, h);
    uint32_t width = static_cast<uint32_t>(w);
    uint32_t height = static_cast<uint32_t>(h);
    m_swapchainExtent.width = std::clamp(width, m_capabilities.minImageExtent.width, m_capabilities.maxImageExtent.width);
    m_swapchainExtent.height = std::clamp(height, m_capabilities.minImageExtent.height, m_capabilities.maxImageExtent.height);    

    m_imageCount = m_capabilities.minImageCount + 1;
    if(m_imageCount > m_capabilities.maxImageCount) m_imageCount = m_capabilities.maxImageCount;
}