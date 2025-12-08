#pragma once
#include "render/PhysicalDevice.h"
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

class PhysicalDevices {
public:
    PhysicalDevices(const VkInstance &instance, const VkSurfaceKHR &surface);
    ~PhysicalDevices() = default;

private:
    std::vector<PhysicalDevice> m_devices;
};