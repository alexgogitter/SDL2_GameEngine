#include "spriteRendererComponent.hpp"

#include <cmath>

#include "object.hpp"
#include "render2D.hpp"

SpriteRendererComponent::SpriteRendererComponent(Object *parentObject, Resource_manager &resourceManager, const char *albedoPath) : Component("SpriteRenderer2D", parentObject), resources(&resourceManager)
{
    if (albedoPath != nullptr) {
        setAlbedoTexture(albedoPath);
    }
}

bool SpriteRendererComponent::setAlbedoTexture(const char *texturePath)
{
    material.albedo = resources->loadTexture(texturePath, TextureColourSpace::SRGB);
    if (material.albedo != InvalidTextureHandle) {
        fitDrawRectToTexture(material.albedo);
    }
    return material.albedo != InvalidTextureHandle;
}

bool SpriteRendererComponent::setAlphaMask(const char *texturePath)
{
    material.alphaMask = resources->loadTexture(texturePath, TextureColourSpace::Linear);
    return material.alphaMask != InvalidTextureHandle;
}

bool SpriteRendererComponent::setNormalMap(const char *texturePath)
{
    material.normal = resources->loadTexture(texturePath, TextureColourSpace::Linear);
    return material.normal != InvalidTextureHandle;
}

bool SpriteRendererComponent::setHeightMap(const char *texturePath)
{
    material.height = resources->loadTexture(texturePath, TextureColourSpace::Linear);
    return material.height != InvalidTextureHandle;
}

bool SpriteRendererComponent::setEmissionMap(const char *texturePath)
{
    material.emission = resources->loadTexture(texturePath, TextureColourSpace::SRGB);
    return material.emission != InvalidTextureHandle;
}

bool SpriteRendererComponent::setDiffuseMap(const char *texturePath)
{
    material.diffuse = resources->loadTexture(texturePath, TextureColourSpace::Linear);
    return material.diffuse != InvalidTextureHandle;
}

bool SpriteRendererComponent::setSpecularMap(const char *texturePath)
{
    material.specular = resources->loadTexture(texturePath, TextureColourSpace::Linear);
    return material.specular != InvalidTextureHandle;
}

void SpriteRendererComponent::setDrawRect(const glm::vec4 &localRectPixels)
{
    drawRectPixels = localRectPixels;
    drawRectConfigured = localRectPixels.z > 0.0f && localRectPixels.w > 0.0f;
    drawRectExplicit = drawRectConfigured;
}

void SpriteRendererComponent::setDrawRect(float x, float y, float width, float height) { setDrawRect({x, y, width, height}); }

void SpriteRendererComponent::setDrawSize(const glm::vec2 &sizePixels) { setDrawRect({sizePixels.x * -0.5f, sizePixels.y * -0.5f, sizePixels.x, sizePixels.y}); }

void SpriteRendererComponent::clearDrawRect()
{
    drawRectPixels = {0.0f, 0.0f, 0.0f, 0.0f};
    drawRectConfigured = false;
    drawRectExplicit = false;
}

void SpriteRendererComponent::Update(std::uint64_t) {}

void SpriteRendererComponent::Draw2D(Renderer2D *renderer)
{
    if (renderer == nullptr || parent == nullptr || resources == nullptr) {
        return;
    }

    renderer->SubmitSprite2D(buildDrawTransform(), resolveMaterial(), parent->getLayer());
}

Material2DRenderState SpriteRendererComponent::resolveMaterial() const
{
    Material2DRenderState result;
    result.albedoTexture = resources->getTexture(material.albedo);
    result.alphaMaskTexture = resources->getTexture(material.alphaMask);
    result.diffuseTexture = resources->getTexture(material.diffuse);
    result.normalTexture = resources->getTexture(material.normal);
    result.heightTexture = resources->getTexture(material.height);
    result.emissionTexture = resources->getTexture(material.emission);
    result.specularTexture = resources->getTexture(material.specular);
    result.albedoUvRect = material.albedoUvRect;
    result.alphaMaskUvRect = material.alphaMaskUvRect;
    result.diffuseUvRect = material.diffuseUvRect;
    result.normalUvRect = material.normalUvRect;
    result.heightUvRect = material.heightUvRect;
    result.emissionUvRect = material.emissionUvRect;
    result.specularUvRect = material.specularUvRect;
    result.tint = material.tint;
    result.normalStrength = material.normalStrength;
    result.heightScale = material.heightScale;
    result.specularStrength = material.specularStrength;
    result.shininess = material.shininess;
    result.lit = material.lit;
    result.renderLayer = material.renderLayer;
    return result;
}

Transform2D SpriteRendererComponent::buildDrawTransform() const
{
    if (parent == nullptr || !drawRectConfigured) {
        return parent != nullptr ? parent->transform.toTransform2D() : Transform2D();
    }
    const Transform3D worldTransform = parent->getWorldTransform();

    Transform2D drawTransform = worldTransform.toTransform2D();
    const glm::vec2 localCentre = {drawRectPixels.x + drawRectPixels.z * 0.5f, drawRectPixels.y + drawRectPixels.w * 0.5f};

    const float rotation = worldTransform.getRotationZ();
    const float cosine = std::cos(rotation);
    const float sine = std::sin(rotation);
    const glm::vec2 rotatedCentre = {localCentre.x * cosine - localCentre.y * sine, localCentre.x * sine + localCentre.y * cosine};

    drawTransform.setPosition(worldTransform.getPosition2D() + rotatedCentre);
    drawTransform.setScale({drawRectPixels.z * 0.5f, drawRectPixels.w * 0.5f});
    return drawTransform;
}

void SpriteRendererComponent::fitDrawRectToTexture(TextureHandle textureHandle)
{
    if (drawRectExplicit || resources == nullptr) {
        return;
    }

    const texture_info *info = resources->getTextureInfo(textureHandle);
    if (info == nullptr || info->width <= 0 || info->height <= 0) {
        return;
    }

    drawRectPixels = {static_cast<float>(info->width) * -0.5f, static_cast<float>(info->height) * -0.5f, static_cast<float>(info->width), static_cast<float>(info->height)};
    drawRectConfigured = true;
}
