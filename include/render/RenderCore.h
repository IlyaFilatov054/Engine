#pragma once
#include "render/Camera.h"
#include "render/FrameManager.h"
#include "render/VkContext.h"
#include "render/Swapchain.h"
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "render/MeshBuffer.h"
#include "render/Camera.h"
#include "render/Vertex.h"

class RenderCore {
public:
    RenderCore(const VkContext* context, const Swapchain* swapchain);
    ~RenderCore();

    void drawFrame();

private:
    const VkContext* m_context = nullptr;
    const Swapchain* m_swapchain = nullptr;
    
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    std::vector<VkShaderModule> m_shaders;
    FrameManager* m_frameManager = nullptr;

    [[deprecated]] MeshBuffer* m_buffer = nullptr; 
    [[deprecated]] std::vector<Vertex> m_vertices {
        Vertex{.position = {-1.0f, -1.0f, -1.0f}, .color = {1.0f, 0.0f, 0.0f, 1.0f}},
        Vertex{.position = {1.0f, -1.0f, -1.0f}, .color = {0.0f, 1.0f, 0.0f, 1.0f}},
        Vertex{.position = {1.0f, 1.0f, -1.0f}, .color = {0.0f, 0.0f, 1.0f, 1.0f}},
        Vertex{.position = {-1.0f, 1.0f, -1.0f}, .color = {1.0f, 1.0f, 1.0f, 1.0f}},
        Vertex{.position = {-1.0f, -1.0f, 1.0f}, .color = {1.0f, 0.0f, 0.0f, 1.0f}},
        Vertex{.position = {1.0f, -1.0f, 1.0f}, .color = {0.0f, 1.0f, 0.0f, 1.0f}},
        Vertex{.position = {1.0f, 1.0f, 1.0f}, .color = {0.0f, 0.0f, 1.0f, 1.0f}},
        Vertex{.position = {-1.0f, 1.0f, 1.0f}, .color = {1.0f, 1.0f, 1.0f, 1.0f}},
    };
    [[deprecated]] std::vector<uint32_t> m_indices {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4};
    [[deprecated]] Camera* camera = nullptr;
    
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet m_cameraDescriptorSet = VK_NULL_HANDLE;

    void createRenderPass();
    VkShaderModule createShaderModule(const std::vector<char> code);
    void createPipeline();
    void recordCommandBuffer(VkCommandBuffer buffer, const VkFramebuffer framebuffer);
    void createDescriptors();
};