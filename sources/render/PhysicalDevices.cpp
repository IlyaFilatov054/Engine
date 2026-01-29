#include "render/PhysicalDevices.h"
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "render/PhysicalDevice.h"
#include "render/VkUtils.h"

PhysicalDevices::PhysicalDevices(const VkInstance &instance, const VkSurfaceKHR &surface){
    uint32_t deviceCount = 0;
    auto res = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    validateVkResult(res, " vkEnumeratePhysicalDevices 1");

    std::vector<VkPhysicalDevice> devices(deviceCount);
    res = vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    validateVkResult(res, " vkEnumeratePhysicalDevices 2");

    for(const auto& d : devices) m_devices.emplace_back(d, instance, surface);
}

const std::vector<const PhysicalDevice*> PhysicalDevices::getDevices() const {
    std::vector<const PhysicalDevice*> result;
    for(const auto& d : m_devices) result.push_back(&d);
    return result;
}

const PhysicalDevice* PhysicalDevices::selected() const {
    return  m_selected;
}

void PhysicalDevices::selectDevice(uint32_t index) {
    m_selected = &m_devices[index];
}