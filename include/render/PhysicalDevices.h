#pragma once
#include "render/PhysicalDevice.h"
#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

class PhysicalDevices {
public:
    PhysicalDevices(const VkInstance &instance, const VkSurfaceKHR &surface);
    ~PhysicalDevices() = default;

    const std::vector<const PhysicalDevice*> getDevices() const;
    const PhysicalDevice* selected() const;
    void selectDevice(uint32_t index);
private:
    std::vector<PhysicalDevice> m_devices;
    PhysicalDevice* m_selected = nullptr;
};