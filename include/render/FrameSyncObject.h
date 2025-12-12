#pragma once
#include <vulkan/vulkan.h>

struct FrameSyncObject {
    VkSemaphore imageAvailable;
    VkSemaphore renderFinished;
    VkFence gpuReady;
};