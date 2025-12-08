#include "render/VkUtils.h"
#include <cstdlib>
#include <iostream>

void validateVkResult(const VkResult &result, const char *message){
    if(result == VK_SUCCESS) return;
    std::cerr << message;
    exit(1);
}