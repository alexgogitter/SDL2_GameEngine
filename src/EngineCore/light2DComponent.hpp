#pragma once

#include "component.hpp"
#include "lighting2D.hpp"

/// Attachable point light whose world XY position follows its parent object.
class Light2DComponent final : public Component
{
  public:
    /// Creates a forward-rendered point light.
    /// @param parent Non-owned object providing world XY position.
    /// @param colour Linear RGB light colour.
    /// @param intensity Non-negative light-energy multiplier.
    /// @param radius Planar attenuation radius in world pixels.
    /// @param height Pseudo-Z height above the 2D surface in world units.
    Light2DComponent(Object *parent, const glm::vec3 &colour = {1.0f, 1.0f, 1.0f}, float intensity = 1.0f, float radius = 300.0f, float height = 100.0f);

    /// Sets linear RGB light colour.
    void setColour(const glm::vec3 &value) { light.colour = value; }
    /// Sets the light-energy multiplier.
    void setIntensity(float value) { light.intensity = value < 0.0f ? 0.0f : value; }
    /// Sets planar attenuation radius in world pixels.
    void setRadius(float value) { light.radius = value < 1.0f ? 1.0f : value; }
    /// Sets pseudo-Z height above the 2D surface.
    void setHeight(float value) { light.position.z = value < 0.0f ? 0.0f : value; }
    /// Marks the light for a future shadow pass. Currently has no visual effect.
    void setCastsShadows(bool value) { light.castsShadows = value; }

    /// @return Read-only local light properties. XY is replaced from the parent when submitted.
    const PointLight2D &getLight() const { return light; }

  private:
    void Update(std::uint64_t deltaTime) override;
    void Draw2D(Renderer2D *renderer) override;
    std::size_t GetPropertyCount() const override;

    bool GetProperty(std::size_t index, ComponentProperty &property) override;

    void OnPropertyChanged(const char *key) override;
    PointLight2D light;
};
