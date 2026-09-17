#pragma once

#include <glm.hpp>

/// World-space point light submitted to the 2D forward renderer.
struct PointLight2D
{
    /// World X/Y and pseudo-height Z.
    glm::vec3 position = {0.0f, 0.0f, 100.0f};
    /// Linear RGB colour multiplier.
    glm::vec3 colour = {1.0f, 1.0f, 1.0f};
    /// Light-energy multiplier.
    float intensity = 1.0f;
    /// Planar attenuation radius in world pixels.
    float radius = 300.0f;
    /// Reserved for the planned shadow pass; currently not consumed.
    bool castsShadows = false;
};
