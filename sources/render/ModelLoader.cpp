#include "render/ModelLoader.h"

MeshData ModelLoader::getCube() const {
    return MeshData {
        .vertices {
            Vertex{.position = {-1.0f, -1.0f, -1.0f}, .color = {1.0f, 1.0f, 1.0f, 1.0f}, .uv = {0.0f, 0.0f}},
            Vertex{.position = {1.0f, -1.0f, -1.0f}, .color = {1.0f, 1.0f, 1.0f, 1.0f}, .uv = {1.0f, 0.0f}},
            Vertex{.position = {1.0f, 1.0f, -1.0f}, .color = {1.0f, 1.0f, 1.0f, 1.0f}, .uv = {1.0f, 1.0f}},
            Vertex{.position = {-1.0f, 1.0f, -1.0f}, .color = {1.0f, 1.0f, 1.0f, 1.0f}, .uv = {0.0f, 1.0f}},
            Vertex{.position = {-1.0f, -1.0f, 1.0f}, .color = {1.0f, 1.0f, 1.0f, 1.0f}, .uv = {0.0f, 0.0f}},
            Vertex{.position = {1.0f, -1.0f, 1.0f}, .color = {1.0f, 1.0f, 1.0f, 1.0f}, .uv = {1.0f, 0.0f}},
            Vertex{.position = {1.0f, 1.0f, 1.0f}, .color = {1.0f, 1.0f, 1.0f, 1.0f}, .uv = {1.0f, 1.0f}},
            Vertex{.position = {-1.0f, 1.0f, 1.0f}, .color = {1.0f, 1.0f, 1.0f, 1.0f}, .uv = {0.0f, 1.0f}},
        },
        .indices {
            4, 5, 6, 6, 7, 4,
            3, 2, 1, 1, 0, 3,
            4, 7, 3, 3, 0, 4,
            5, 1, 2, 2, 6, 5,
            7, 6, 2, 2, 3, 7,
            0, 1, 5, 5, 4, 0
        }
    };
}