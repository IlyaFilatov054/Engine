#include "render/Camera.h"
#include <glm/fwd.hpp>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(const VkContext* context)
 : m_context(context) {

}

Camera::~Camera() {

}

uint32_t Camera::dataSize() const {
    return sizeof(glm::mat4) * 2;
}

void* Camera::data() {
    m_view = glm::lookAt(position, target, up);
    m_projection = glm::perspective(glm::radians(fov), aspect, near, far);
    m_projection[1][1] *= -1;
    m_data.clear();
    m_data = {m_view, m_projection};
    return m_data.data();
}