#include "render/VkUtils.h"
#include <cstdlib>
#include <iostream>
#include <vulkan/vulkan_core.h>

void validateVkResult(const VkResult &result, const char *message){
    if(result == VK_SUCCESS) return;
    std::cerr << result << '\n';
    vkRendererError(message);
}

void vkRendererError(const char* message) {
    std::cerr << message << '\n';
    exit(1);
}

void executeOnGpu(const VkContext* context, std::function<void(const VkCommandBuffer commandBuffer)> function) {
    VkCommandPool pool = VK_NULL_HANDLE;
    VkCommandPoolCreateInfo commandPoolInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = context->graphicsQueueIndex(),
    };
    vkCreateCommandPool(context->device(), &commandPoolInfo, nullptr, &pool);

    VkCommandBufferAllocateInfo commandBufferAllocateInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(context->device(), &commandBufferAllocateInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    function(commandBuffer);
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submit {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer
    };
    vkQueueSubmit(context->graphicsQueue(), 1, &submit, nullptr);
    vkQueueWaitIdle(context->graphicsQueue());

    vkFreeCommandBuffers(context->device(), pool, 1, &commandBuffer);
    vkDestroyCommandPool(context->device(), pool, nullptr);
}