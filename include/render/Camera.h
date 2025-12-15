#pragma once

#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include "render/Buffer.h"
#include "render/VkContext.h"

class Camera {
public:
    Camera(const VkContext* context, const VkDescriptorSet descriptorSet);
    ~Camera();
private:
    glm::mat4 m_view;
    glm::mat4 m_projection;
    Buffer m_buffer;
    const VkContext* m_context;
    const VkDescriptorSet m_descriptorSet;
    VkWriteDescriptorSet m_write;

    void updateData();
};