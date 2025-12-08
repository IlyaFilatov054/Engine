#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

class QueueFamily {
public:
    QueueFamily(uint32_t index, const VkQueueFamilyProperties &properties, const VkPhysicalDevice &device, const VkSurfaceKHR &surface);
    ~QueueFamily() = default;

    uint32_t index() const;
    const VkQueueFamilyProperties properties() const;
    bool presentSupport() const;
    bool checkFlags(const VkQueueFlags &flags) const;
private:
    uint32_t m_index;
    VkBool32 m_presentSupport = false;
    VkQueueFamilyProperties m_properties;
};