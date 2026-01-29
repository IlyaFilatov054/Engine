#include "render/PhysicalDevice.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "render/QueueFamily.h"
#include "render/VkUtils.h"

PhysicalDevice::PhysicalDevice(const VkPhysicalDevice& device, const VkInstance &instance, const VkSurfaceKHR &surface) 
    : m_device(device)
{
    vkGetPhysicalDeviceProperties(m_device, &m_properties);
    vkGetPhysicalDeviceFeatures(m_device, &m_features);
    vkGetPhysicalDeviceMemoryProperties(m_device, &m_memoryProperties);

    uint32_t queueFamiliesCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliesCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamiliesCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliesCount, queueFamilies.data());
    for(size_t i = 0; i < queueFamilies.size(); i++) m_queueFamilies.emplace_back(i, queueFamilies[i], device, surface);

    uint32_t extensionCount = 0;
    auto res = vkEnumerateDeviceExtensionProperties(m_device, nullptr, &extensionCount, nullptr);
    validateVkResult(res, "vkEnumerateDeviceExtensionProperties 1");
    m_extensionProperties.resize(extensionCount);
    res = vkEnumerateDeviceExtensionProperties(m_device, nullptr, &extensionCount, m_extensionProperties.data());
    validateVkResult(res, "vkEnumerateDeviceExtensionProperties 1");
}

const VkPhysicalDevice& PhysicalDevice::device() const {
    return  m_device;
}

std::vector<const QueueFamily*> PhysicalDevice::getQueues(const VkBool32 &presentSupport, const VkQueueFlags &flags) const {
    std::vector<const QueueFamily*> result;
    for(const auto& q : m_queueFamilies){
        if(q.presentSupport() != presentSupport) continue;
        if(!q.checkFlags(flags)) continue;
        result.push_back(&q);
    }
    return  result;;
}

bool PhysicalDevice::hasExtension(const char* name) const {
    for(const auto& e : m_extensionProperties){
        if(strcmp(e.extensionName, name) == 0) return  true;
    }
    return false;
}