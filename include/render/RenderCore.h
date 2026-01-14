#pragma once
#include "render/Camera.h"
#include "render/DescriptorManager.h"
#include "render/FrameManager.h"
#include "render/FrameResources.h"
#include "render/ImageResources.h"
#include "render/RenderPass.h"
#include "render/ResourceManager.h"
#include "render/ShaderManager.h"
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

    RenderPass* m_renderPass = nullptr;
    ShaderManager* m_shaderManager = nullptr;
    FrameManager* m_frameManager = nullptr;
    DescriptorManager* m_descriptorManager = nullptr;
    ResourceManager* m_resourceManager = nullptr;
 
    [[deprecated]] std::vector<Vertex> m_vertices {
        Vertex{.position = {-1.0f, -1.0f, -1.0f}, .color = {1.0f, 1.0f, 1.0f, 1.0f}, .uv = {0.0f, 0.0f}},
        Vertex{.position = {1.0f, -1.0f, -1.0f}, .color = {1.0f, 1.0f, 1.0f, 1.0f}, .uv = {0.0f, 1.0f}},
        Vertex{.position = {1.0f, 1.0f, -1.0f}, .color = {1.0f, 1.0f, 1.0f, 1.0f}, .uv = {1.0f, 0.0f}},
        Vertex{.position = {-1.0f, 1.0f, -1.0f}, .color = {1.0f, 1.0f, 1.0f, 1.0f}, .uv = {1.0f, 1.0f}},
        Vertex{.position = {-1.0f, -1.0f, 1.0f}, .color = {1.0f, 1.0f, 1.0f, 1.0f}, .uv = {0.0f, 0.0f}},
        Vertex{.position = {1.0f, -1.0f, 1.0f}, .color = {1.0f, 1.0f, 1.0f, 1.0f}, .uv = {0.0f, 1.0f}},
        Vertex{.position = {1.0f, 1.0f, 1.0f}, .color = {1.0f, 1.0f, 1.0f, 1.0f}, .uv = {1.0f, 0.0f}},
        Vertex{.position = {-1.0f, 1.0f, 1.0f}, .color = {1.0f, 1.0f, 1.0f, 1.0f}, .uv = {1.0f, 1.0f}},
    };
    [[deprecated]] std::vector<uint32_t> m_indices {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4};
    [[deprecated]] Camera* camera = nullptr;

    void recordCommandBuffer(const FrameResources& frameResources, const ImageResources& imageResources);
};