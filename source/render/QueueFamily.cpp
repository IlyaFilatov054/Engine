#include "render/QueueFamily.h"
#include <cstdint>
#include "render/VkUtils.h"

QueueFamily::QueueFamily(uint32_t index, const VkQueueFamilyProperties &properties, const VkPhysicalDevice& device, const VkSurfaceKHR &surface) 
    : m_index(index), m_properties(properties)
{
    auto res = vkGetPhysicalDeviceSurfaceSupportKHR(device, m_index, surface, &m_presentSupport);
    validateVkResult(res, "vkGetPhysicalDeviceSurfaceSupportKHR");
}

uint32_t QueueFamily::index() const {
    return m_index;
}

bool QueueFamily::presentSupport() const {
    return m_presentSupport;
}

bool QueueFamily::checkFlags(const VkQueueFlags &flags) const {
    return m_properties.queueFlags & flags;
}
