#pragma once
#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "render/VkContext.h"

class Swapchain {
public:
    Swapchain(const VkContext* context);
    ~Swapchain();

    const VkSurfaceFormatKHR& format() const;
    const std::vector<VkImageView>& imageViews() const;
    const VkExtent2D& extent() const;    
private:
    const VkContext* m_context = nullptr;
    VkSurfaceCapabilitiesKHR m_capabilities;
    std::vector<VkSurfaceFormatKHR> m_formats;
    VkSurfaceFormatKHR m_selectedFormat;
    std::vector<VkPresentModeKHR> m_presentModes;
    VkPresentModeKHR m_selectedPresentMode;
    VkExtent2D m_swapchainExtent;
    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_imageViews;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;

    void initCapabilities();
    void initFormat();
    void initPresentMode();
    void initExtent();
    void createSwapchain();
    void createImages();
    void createImageViews();
};