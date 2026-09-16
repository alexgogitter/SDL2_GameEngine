#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>

#include <glm/glm.hpp>

using MeshHandle = std::uint64_t;

struct MeshRenderState
{
    unsigned int vertexArray = 0;
    std::size_t indexCount = 0;
};

constexpr MeshHandle InvalidMeshHandle =
    std::numeric_limits<MeshHandle>::max();

struct MeshVertex
{
    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::vec3 normal = {0.0f, 1.0f, 0.0f};
    glm::vec2 textureCoordinate = {0.0f, 0.0f};
};