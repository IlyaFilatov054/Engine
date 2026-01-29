#pragma once
#include "render/Camera.h"
#include "render/DescriptorManager.h"
#include "render/FrameManager.h"
#include "render/RenderGraph.h"
#include "render/RenderObject.h"
#include "render/ResourceManager.h"
#include "render/ShaderManager.h"
#include "render/VkContext.h"
#include "render/Swapchain.h"
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>
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

    RenderGraph* m_renderGraph = nullptr;
    ShaderManager* m_shaderManager = nullptr;
    FrameManager* m_frameManager = nullptr;
    DescriptorManager* m_descriptorManager = nullptr;
    ResourceManager* m_resourceManager = nullptr;
 
    std::vector<RenderObject> m_renderObjects;
    [[deprecated]] std::vector<Vertex> m_vertices {
        Vertex{.position = {-1.0f, -1.0f, -1.0f}, .color = {1.0f, 1.0f, 1.0f, 1.0f}, .uv = {0.0f, 0.0f}},
        Vertex{.position = {1.0f, -1.0f, -1.0f}, .color = {1.0f, 1.0f, 1.0f, 1.0f}, .uv = {1.0f, 0.0f}},
        Vertex{.position = {1.0f, 1.0f, -1.0f}, .color = {1.0f, 1.0f, 1.0f, 1.0f}, .uv = {1.0f, 1.0f}},
        Vertex{.position = {-1.0f, 1.0f, -1.0f}, .color = {1.0f, 1.0f, 1.0f, 1.0f}, .uv = {0.0f, 1.0f}},
        Vertex{.position = {-1.0f, -1.0f, 1.0f}, .color = {1.0f, 1.0f, 1.0f, 1.0f}, .uv = {0.0f, 0.0f}},
        Vertex{.position = {1.0f, -1.0f, 1.0f}, .color = {1.0f, 1.0f, 1.0f, 1.0f}, .uv = {1.0f, 0.0f}},
        Vertex{.position = {1.0f, 1.0f, 1.0f}, .color = {1.0f, 1.0f, 1.0f, 1.0f}, .uv = {1.0f, 1.0f}},
        Vertex{.position = {-1.0f, 1.0f, 1.0f}, .color = {1.0f, 1.0f, 1.0f, 1.0f}, .uv = {0.0f, 1.0f}},
    };
    [[deprecated]] std::vector<uint32_t> m_indices {
        4, 5, 6, 6, 7, 4,
        3, 2, 1, 1, 0, 3,
        4, 7, 3, 3, 0, 4,
        5, 1, 2, 2, 6, 5,
        7, 6, 2, 2, 3, 7,
        0, 1, 5, 5, 4, 0
    };
    [[deprecated]] Camera* camera1 = nullptr;
    [[deprecated]] Camera* camera2 = nullptr;

    void recordCommandBuffer(const VkCommandBuffer buffer, uint32_t image);
};