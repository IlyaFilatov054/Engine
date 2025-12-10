#include "render/Swapchain.h"
#include <algorithm>
#include <cstdint>
#include <string>
#include <vulkan/vulkan_core.h>
#include "render/VkUtils.h"

Swapchain::Swapchain(const VkContext* context) {
    m_context = context;
    auto res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_context->physicalDevice(), m_context->surface(), &m_capabilities);
    validateVkResult(res, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");

    uint32_t formatCount;
    res = vkGetPhysicalDeviceSurfaceFormatsKHR(m_context->physicalDevice(), m_context->surface(), &formatCount, nullptr);
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

    uint32_t presentModeCount;
    res = vkGetPhysicalDeviceSurfacePresentModesKHR(m_context->physicalDevice(), m_context->surface(), &presentModeCount, nullptr);
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

    int w, h;
    m_context->window()->getSize(w, h);
    uint32_t width = static_cast<uint32_t>(w);
    uint32_t height = static_cast<uint32_t>(h);
    m_swapchainExtent.width = std::clamp(width, m_capabilities.minImageExtent.width, m_capabilities.maxImageExtent.width);
    m_swapchainExtent.height = std::clamp(height, m_capabilities.minImageExtent.height, m_capabilities.maxImageExtent.height);    

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
    res = vkCreateSwapchainKHR(m_context->device(), &createInfo, nullptr, &m_swapchain);
    validateVkResult(res, "vkCreateSwapchainKHR");

    uint32_t swapchainImageCount;
    res = vkGetSwapchainImagesKHR(m_context->device(), m_swapchain, &swapchainImageCount, nullptr);
    validateVkResult(res, "vkGetSwapchainImagesKHR 1");
    m_images.resize(swapchainImageCount);
    res = vkGetSwapchainImagesKHR(m_context->device(), m_swapchain, &swapchainImageCount, m_images.data());
    validateVkResult(res, "vkGetSwapchainImagesKHR 2");

    m_imageViews.resize(swapchainImageCount);
    for(uint32_t i = 0; i < swapchainImageCount; i++){
        VkImageViewCreateInfo imageViewCreateInfo {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = m_images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = m_selectedFormat.format,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
        };
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;

        res = vkCreateImageView(m_context->device(), &imageViewCreateInfo, nullptr, &m_imageViews[i]);
        validateVkResult(res, ("vkCreateImageView" + std::to_string(i)).data());
    }
}

Swapchain::~Swapchain(){
    vkDeviceWaitIdle(m_context->device());
    for(auto view : m_imageViews){
        vkDestroyImageView(m_context->device(), view, nullptr);
    }
    vkDestroySwapchainKHR(m_context->device(), m_swapchain, nullptr);
}