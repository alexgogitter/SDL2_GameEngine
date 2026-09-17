#pragma once

#include <limits>

#include <glm.hpp>

using TextureHandle = unsigned int;
/// Sentinel returned when a texture resource could not be loaded.
inline constexpr TextureHandle InvalidTextureHandle = std::numeric_limits<TextureHandle>::max();

/// Component-facing 2D material using stable resource handles.
/// Resource handles stay backend-independent and are resolved during submission.
struct Material2D
{
    /// sRGB base-colour and opacity texture.
    TextureHandle albedo = InvalidTextureHandle;
    /// Linear grayscale alpha mask multiplied into albedo alpha.
    TextureHandle alphaMask = InvalidTextureHandle;
    /// Linear roughness/matte-response map; white is rough/diffuse, black is smooth.
    TextureHandle diffuse = InvalidTextureHandle;
    /// Linear tangent-space normal texture.
    TextureHandle normal = InvalidTextureHandle;
    /// Linear grayscale pseudo-height texture.
    TextureHandle height = InvalidTextureHandle;
    /// sRGB self-illumination colour texture.
    TextureHandle emission = InvalidTextureHandle;
    /// Linear shininess/specular intensity map for point-light highlights.
    TextureHandle specular = InvalidTextureHandle;

    /// Normalized source rectangle for albedo sampling: x, y, width, height.
    glm::vec4 albedoUvRect = {0.0f, 0.0f, 1.0f, 1.0f};
    /// Normalized source rectangle for alpha-mask sampling: x, y, width, height.
    glm::vec4 alphaMaskUvRect = {0.0f, 0.0f, 1.0f, 1.0f};
    /// Normalized source rectangle for diffuse/roughness sampling: x, y, width, height.
    glm::vec4 diffuseUvRect = {0.0f, 0.0f, 1.0f, 1.0f};
    /// Normalized source rectangle for normal-map sampling: x, y, width, height.
    glm::vec4 normalUvRect = {0.0f, 0.0f, 1.0f, 1.0f};
    /// Normalized source rectangle for height-map sampling: x, y, width, height.
    glm::vec4 heightUvRect = {0.0f, 0.0f, 1.0f, 1.0f};
    /// Normalized source rectangle for emission-map sampling: x, y, width, height.
    glm::vec4 emissionUvRect = {0.0f, 0.0f, 1.0f, 1.0f};
    /// Normalized source rectangle for specular-map sampling: x, y, width, height.
    glm::vec4 specularUvRect = {0.0f, 0.0f, 1.0f, 1.0f};

    /// Linear RGBA multiplier applied to albedo.
    glm::vec4 tint = {1.0f, 1.0f, 1.0f, 1.0f};
    /// Multiplier applied to tangent-space normal XY components.
    float normalStrength = 1.0f;
    /// Maximum pseudo-height represented by a white height-map texel.
    float heightScale = 0.0f;
    /// Multiplier applied to sampled specular/shiny intensity.
    float specularStrength = 1.0f;
    /// Maximum Blinn-Phong highlight exponent used by white specular-map texels.
    float shininess = 32.0f;
    /// Whether ambient and submitted point lights affect the material.
    bool lit = true;
    /// Stable primary sort key; larger layers render later.
    int renderLayer = 0;
};

/// Renderer-facing material state containing resolved GPU texture object names.
struct Material2DRenderState
{
    unsigned int albedoTexture = 0;
    unsigned int alphaMaskTexture = 0;
    unsigned int diffuseTexture = 0;
    unsigned int normalTexture = 0;
    unsigned int heightTexture = 0;
    unsigned int emissionTexture = 0;
    unsigned int specularTexture = 0;

    glm::vec4 albedoUvRect = {0.0f, 0.0f, 1.0f, 1.0f};
    glm::vec4 alphaMaskUvRect = {0.0f, 0.0f, 1.0f, 1.0f};
    glm::vec4 diffuseUvRect = {0.0f, 0.0f, 1.0f, 1.0f};
    glm::vec4 normalUvRect = {0.0f, 0.0f, 1.0f, 1.0f};
    glm::vec4 heightUvRect = {0.0f, 0.0f, 1.0f, 1.0f};
    glm::vec4 emissionUvRect = {0.0f, 0.0f, 1.0f, 1.0f};
    glm::vec4 specularUvRect = {0.0f, 0.0f, 1.0f, 1.0f};

    glm::vec4 tint = {1.0f, 1.0f, 1.0f, 1.0f};
    float normalStrength = 1.0f;
    float heightScale = 0.0f;
    float specularStrength = 1.0f;
    float shininess = 32.0f;
    bool lit = true;
    int renderLayer = 0;
};
