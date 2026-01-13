#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <glm/fwd.hpp>

struct RenderObjectData {
    glm::mat4 model;
    uint32_t textureIndex;
    uint32_t padding[3];
};