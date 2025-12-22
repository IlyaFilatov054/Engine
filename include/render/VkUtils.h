#pragma once

#include "render/VkContext.h"
#include <functional>
#include <vulkan/vulkan.h>

void validateVkResult(const VkResult& result, const char* message);
void vkRendererError(const char* message);
void executeOnGpu(const VkContext* context, const VkCommandPool pool, std::function<void(const VkCommandBuffer commandBuffer)> function);