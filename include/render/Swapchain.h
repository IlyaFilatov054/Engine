#pragma once
#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "render/Window.h"

class Swapchain {
public:
    Swapchain(const VkPhysicalDevice device, const VkSurfaceKHR surface, const Window* window);
    ~Swapchain();

private:
    VkSurfaceCapabilitiesKHR m_capabilities;
    std::vector<VkSurfaceFormatKHR> m_formats;
    VkSurfaceFormatKHR m_selectedFormat;
    std::vector<VkPresentModeKHR> m_presentModes;
    VkPresentModeKHR m_selectedPresentMode;
    VkExtent2D m_swapchainExtent;
    uint32_t m_imageCount;
};