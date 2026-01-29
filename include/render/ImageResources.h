#pragma once

#include "render/VkContext.h"
#include <vulkan/vulkan_core.h>

class ImageResources {
public:
    ImageResources(const VkContext* context);
    ~ImageResources();

    const VkSemaphore& renderFinishedSemaphore() const;
private:
    const VkContext* m_context;

    VkSemaphore m_renderFinished = VK_NULL_HANDLE;

    void createSemaphore();
};