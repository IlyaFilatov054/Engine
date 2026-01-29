#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "render/Image.h"
#include "render/VkContext.h"

class Swapchain {
public:
    Swapchain(const VkContext* context);
    ~Swapchain();

    const VkSurfaceFormatKHR& format() const;
    const std::vector<Image*>& images() const;
    const VkExtent2D& extent() const;
    const VkSwapchainKHR& swapchain() const;    
private:
    const VkContext* m_context = nullptr;
    VkSurfaceCapabilitiesKHR m_capabilities;
    std::vector<VkSurfaceFormatKHR> m_formats;
    VkSurfaceFormatKHR m_selectedFormat;
    std::vector<VkPresentModeKHR> m_presentModes;
    VkPresentModeKHR m_selectedPresentMode;
    VkExtent2D m_swapchainExtent;
    std::vector<Image*> m_images;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;

    void initCapabilities();
    void initFormat();
    void initPresentMode();
    void initExtent();
    void createSwapchain();
    void createImages();
};