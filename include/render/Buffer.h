#pragma once
#include <cstdint>
#include <stdexcept>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "render/VkContext.h"
#include "render/AbstractBuffer.h"

class Buffer : public AbstractBuffer{
public:
    Buffer(const VkContext* context, uint32_t size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryFlags);
    ~Buffer() override;

    const VkBuffer& buffer() const override;
    void setData(void* data) override {throw std::runtime_error("Can't set data to default buffer!");};
    
protected:
    VkBuffer m_buffer;
    VkDeviceMemory m_memory;

private:
    void createBuffer(VkBufferUsageFlags usageFlags);
    void allocateMemory(VkMemoryPropertyFlags memoryFlags);
};