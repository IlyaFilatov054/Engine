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
    
    [[deprecated]] Camera* camera1 = nullptr;
    [[deprecated]] Camera* camera2 = nullptr;

    void recordCommandBuffer(const VkCommandBuffer buffer, uint32_t image);
};