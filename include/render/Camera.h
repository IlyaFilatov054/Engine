#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "render/MappedBuffer.h"
#include "render/VkContext.h"

class Camera {
public:
    Camera(const VkContext* context);
    ~Camera();

    glm::vec3 position{};
    glm::vec3 target{0.f, 0.f, 0.f};
    glm::vec3 up{0.f, 1.f, 0.f};
    float fov{60.f};
    float aspect{4.f / 3.f};
    float near{0.01f};
    float far{100.0f};

    uint32_t dataSize() const;
    void* data();
private:
    const VkContext* m_context;
    glm::mat4 m_view;
    glm::mat4 m_projection;
    std::vector<glm::mat4> m_data;
};