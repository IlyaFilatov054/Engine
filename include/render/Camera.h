#pragma once

#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include "render/Buffer.h"
#include "render/VkContext.h"

class Camera {
public:
    Camera(const VkContext* context, const VkDescriptorSet descriptorSet);
    ~Camera();

    glm::vec3 position{};
    glm::vec3 target{0.f, 0.f, 0.f};
    glm::vec3 up{0.f, 1.f, 0.f};
    float fov{60.f};
    float aspect{4.f / 3.f};
    float near{0.01f};
    float far{100.0f};

    void update();
private:
    glm::mat4 m_view;
    glm::mat4 m_projection;
    Buffer m_buffer;
    const VkContext* m_context;
    const VkDescriptorSet m_descriptorSet;
};