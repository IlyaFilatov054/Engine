#pragma once

#include <vulkan/vulkan.h>

void validateVkResult(const VkResult& result, const char* message);
void vkRendererError(const char* message);