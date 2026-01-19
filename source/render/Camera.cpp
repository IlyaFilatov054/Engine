#include "render/Camera.h"
#include <glm/fwd.hpp>
#include <vulkan/vulkan_core.h>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(const VkContext* context, const VkDescriptorSet descriptorSet)
 : m_context(context),
 m_descriptorSet(descriptorSet),
 m_buffer(context, sizeof(glm::mat4) * 2,
VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
    VkDescriptorBufferInfo bufferInfo {
        .buffer = m_buffer.buffer(),
        .offset = 0,
        .range = sizeof(glm::mat4) * 2,
    };

    VkWriteDescriptorSet write {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = m_descriptorSet,
        .dstBinding = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = &bufferInfo,
    };
    vkUpdateDescriptorSets(m_context->device(), 1, &write, 0, nullptr);    
}

Camera::~Camera() {

}

void Camera::update() {
    m_view = glm::lookAt(position, target, up);
    m_projection = glm::perspective(glm::radians(fov), aspect, near, far);
    glm::mat4 data[] = {m_view, m_projection};
    m_buffer.setData(data);
}

const VkDescriptorSet& Camera::descriptorSet() const {
    return m_descriptorSet;
}