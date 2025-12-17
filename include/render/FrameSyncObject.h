#pragma once
#include <vulkan/vulkan.h>

struct FrameSyncObject {
    VkSemaphore imageAvailable;
    VkFence gpuExecuted;
};