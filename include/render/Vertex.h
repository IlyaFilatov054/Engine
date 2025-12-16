#pragma once
#include <array>
#include <cstddef>
#include <vulkan/vulkan_core.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>


struct Vertex {
    glm::vec3 position;
    glm::vec4 color;

    static VkVertexInputBindingDescription bindingDescription() {
        return VkVertexInputBindingDescription {
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        };
    };

    static std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attribs;
        attribs[0] = {
            .location = 0,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, position),
        };
        attribs[1] = {
            .location = 1,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32A32_SFLOAT,
            .offset = offsetof(Vertex, color)
        };
        return  attribs;
    };
};