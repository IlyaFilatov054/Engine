#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "render/QueueFamily.h"

class PhysicalDevice {
public:
    PhysicalDevice(const VkPhysicalDevice &device, const VkInstance &instance, const VkSurfaceKHR &surface);
    ~PhysicalDevice() = default;

    const VkPhysicalDevice& device() const;
    std::vector<const QueueFamily*> getQueues(const VkBool32 &presentSupport, const VkQueueFlags &flags) const;
    bool hasExtension(const char* name) const;
private:
    std::vector<QueueFamily> m_queueFamilies;
    VkPhysicalDeviceProperties m_properties;
    VkPhysicalDeviceFeatures m_features;
    VkPhysicalDeviceMemoryProperties m_memoryProperties;
    VkPhysicalDevice m_device;
    std::vector<VkExtensionProperties> m_extensionProperties;
};