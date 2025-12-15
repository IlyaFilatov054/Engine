#include "render/VkUtils.h"
#include <cstdlib>
#include <iostream>

void validateVkResult(const VkResult &result, const char *message){
    if(result == VK_SUCCESS) return;
    std::cerr << result << '\n';
    vkRendererError(message);
}

void vkRendererError(const char* message) {
    std::cerr << message << '\n';
    exit(1);
}