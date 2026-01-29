#pragma once

#include "render/Vertex.h"
#include <cstdint>
#include <vector>
struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

class ModelLoader {
public:
    ModelLoader() = default;
    ~ModelLoader() = default;

    MeshData getCube() const;
};