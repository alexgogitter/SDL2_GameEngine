#pragma once

#include <glm.hpp>

#include "component.hpp"
#include "material2D.hpp"
#include "resource_manager.hpp"
#include "transform.hpp"

/// Renderer-facing sprite and 2D material component.
///
/// Texture handles are owned by Resource_manager. Draw() resolves the handles
/// and submits a command; it never issues raw OpenGL calls.
class SpriteRendererComponent : public Component
{
  public:
    /// Creates a component and optionally loads its albedo texture.
    /// @param parent Non-owned object whose transform drives the sprite.
    /// @param resources Resource cache that must outlive this component.
    /// @param albedoPath Optional sRGB albedo path.
    SpriteRendererComponent(Object *parent, Resource_manager &resources, const char *albedoPath = nullptr);

    /// Loads and assigns an sRGB base-colour/alpha texture.
    /// @return True when the texture was loaded or retrieved from cache.
    bool setAlbedoTexture(const char *texturePath);
    /// Loads and assigns a linear grayscale alpha mask.
    /// @return True when the texture was loaded or retrieved from cache.
    bool setAlphaMask(const char *texturePath);
    /// Loads and assigns a linear tangent-space normal map.
    /// @return True when the texture was loaded or retrieved from cache.
    bool setNormalMap(const char *texturePath);
    /// Loads and assigns a linear grayscale pseudo-height map.
    /// @return True when the texture was loaded or retrieved from cache.
    bool setHeightMap(const char *texturePath);
    /// Loads and assigns an sRGB self-illumination colour map.
    /// @return True when the texture was loaded or retrieved from cache.
    bool setEmissionMap(const char *texturePath);
    /// Loads and assigns a linear roughness/matte-response map.
    /// White texels behave rough and diffuse; black texels behave smoother.
    /// @return True when the texture was loaded or retrieved from cache.
    bool setDiffuseMap(const char *texturePath);
    /// Loads and assigns a linear RGB specular intensity/colour map.
    /// @return True when the texture was loaded or retrieved from cache.
    bool setSpecularMap(const char *texturePath);
    /// Compatibility alias for setAlbedoTexture().
    bool setTexture(const char *texturePath) { return setAlbedoTexture(texturePath); }
    /// Detaches the albedo handle without deleting the cached resource.
    void clearTexture() { material.albedo = InvalidTextureHandle; }
    /// @return Whether an albedo resource handle is assigned.
    bool hasTexture() const { return material.albedo != InvalidTextureHandle; }

    /// Sets the local pixel rectangle used to draw this sprite.
    ///
    /// The rectangle uses SDL-style fields in parent-local pixels:
    /// x and y are the top-left corner relative to the parent transform centre,
    /// width and height are the full rendered size. The rect follows parent
    /// position and rotation but does not affect the collider.
    ///
    /// @param localRectPixels Local x, y, width and height in pixels.
    void setDrawRect(const glm::vec4 &localRectPixels);

    /// Sets the local pixel rectangle used to draw this sprite.
    /// @param x Local top-left X relative to the parent centre.
    /// @param y Local top-left Y relative to the parent centre.
    /// @param width Full rendered width in pixels.
    /// @param height Full rendered height in pixels.
    void setDrawRect(float x, float y, float width, float height);

    /// Sets a centred draw rectangle with the supplied full pixel size.
    /// @param sizePixels Full rendered width and height in pixels.
    void setDrawSize(const glm::vec2 &sizePixels);

    /// Clears the explicit draw rectangle and lets the next albedo texture size
    /// or parent transform provide the fallback rendered size.
    void clearDrawRect();

    /// @return Local x, y, width and height used for sprite rendering.
    const glm::vec4 &getDrawRect() const { return drawRectPixels; }

    /// @return Whether this component has a texture-sized or explicit draw rect.
    bool hasDrawRect() const { return drawRectConfigured; }

    /// @return Mutable component-owned material configuration.
    Material2D &getMaterial() { return material; }
    /// @return Read-only component-owned material configuration.
    const Material2D &getMaterial() const { return material; }

  private:
    void Update(std::uint64_t deltaTime) override;
    void Draw2D(Renderer2D *renderer) override;
    Material2DRenderState resolveMaterial() const;
    Transform2D buildDrawTransform() const;
    void fitDrawRectToTexture(TextureHandle textureHandle);

    Resource_manager *resources = nullptr;
    Material2D material;
    glm::vec4 drawRectPixels = {0.0f, 0.0f, 0.0f, 0.0f};
    bool drawRectConfigured = false;
    bool drawRectExplicit = false;
};
