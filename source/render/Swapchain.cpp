#include "render/Swapchain.h"
#include <algorithm>
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "render/Image.h"
#include "render/VkUtils.h"

Swapchain::Swapchain(const VkContext* context) {
    m_context = context;

    initCapabilities();
    initFormat();
    initPresentMode();
    initExtent();
    createSwapchain();
    createImages();
}

Swapchain::~Swapchain(){
    vkDeviceWaitIdle(m_context->device());
    for(auto& i : m_images) {
        delete i;
    }
    vkDestroySwapchainKHR(m_context->device(), m_swapchain, nullptr);
}

const VkSurfaceFormatKHR& Swapchain::format() const{
    return  m_selectedFormat;
}

const std::vector<Image*>& Swapchain::images() const {
    return  m_images;
}

const VkExtent2D& Swapchain::extent() const {
    return  m_swapchainExtent;
}

const VkSwapchainKHR& Swapchain::swapchain() const {
    return  m_swapchain;
}

void Swapchain::initCapabilities() {
    auto res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_context->physicalDevice(), m_context->surface(), &m_capabilities);
    validateVkResult(res, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
}

void Swapchain::initFormat() {
    uint32_t formatCount;
    auto res = vkGetPhysicalDeviceSurfaceFormatsKHR(m_context->physicalDevice(), m_context->surface(), &formatCount, nullptr);
    validateVkResult(res, "vkGetPhysicalDeviceSurfaceFormatsKHR 1");
    m_formats.resize(formatCount);
    res = vkGetPhysicalDeviceSurfaceFormatsKHR(m_context->physicalDevice(), m_context->surface(), &formatCount, m_formats.data());
    validateVkResult(res, "vkGetPhysicalDeviceSurfaceFormatsKHR 2");
    m_selectedFormat = m_formats[0];
    for(const auto& f : m_formats){
        if(f.format == VK_FORMAT_B8G8R8A8_SRGB && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
            m_selectedFormat = f;
            break;
        }
    }
}

void Swapchain::initPresentMode() {
    uint32_t presentModeCount;
    auto res = vkGetPhysicalDeviceSurfacePresentModesKHR(m_context->physicalDevice(), m_context->surface(), &presentModeCount, nullptr);
    validateVkResult(res, "vkGetPhysicalDeviceSurfacePresentModesKHR 1");
    m_presentModes.resize(presentModeCount);
    res = vkGetPhysicalDeviceSurfacePresentModesKHR(m_context->physicalDevice(), m_context->surface(), &presentModeCount, m_presentModes.data());
    validateVkResult(res, "vkGetPhysicalDeviceSurfacePresentModesKHR 2");
    m_selectedPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    for(const auto& m : m_presentModes){
        if(m == VK_PRESENT_MODE_MAILBOX_KHR){
            m_selectedPresentMode = m;
            break;
        }
    }
}

void Swapchain::initExtent() {
    int w, h;
    m_context->window()->getSize(w, h);
    uint32_t width = static_cast<uint32_t>(w);
    uint32_t height = static_cast<uint32_t>(h);
    m_swapchainExtent.width = std::clamp(width, m_capabilities.minImageExtent.width, m_capabilities.maxImageExtent.width);
    m_swapchainExtent.height = std::clamp(height, m_capabilities.minImageExtent.height, m_capabilities.maxImageExtent.height);    
}

void Swapchain::createSwapchain() {
    uint32_t imageCount = m_capabilities.minImageCount + 1;
    if(m_capabilities.maxImageCount > 0 && imageCount > m_capabilities.maxImageCount) imageCount = m_capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = m_context->surface(),
        .minImageCount = imageCount,
        .imageFormat = m_selectedFormat.format,
        .imageColorSpace = m_selectedFormat.colorSpace,
        .imageExtent = m_swapchainExtent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = m_capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = m_selectedPresentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE
    };
    auto res = vkCreateSwapchainKHR(m_context->device(), &createInfo, nullptr, &m_swapchain);
    validateVkResult(res, "vkCreateSwapchainKHR");
}

void Swapchain::createImages() {
    uint32_t swapchainImageCount;
    std::vector<VkImage> images;
    auto res = vkGetSwapchainImagesKHR(m_context->device(), m_swapchain, &swapchainImageCount, nullptr);
    validateVkResult(res, "vkGetSwapchainImagesKHR 1");
    images.resize(swapchainImageCount);
    res = vkGetSwapchainImagesKHR(m_context->device(), m_swapchain, &swapchainImageCount, images.data());
    validateVkResult(res, "vkGetSwapchainImagesKHR 2");
    for(auto& i : images) {
        m_images.push_back(new Image(m_context, m_selectedFormat.format, ImageType::Color, i));
    }
}