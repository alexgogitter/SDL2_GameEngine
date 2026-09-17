#pragma once

#include <cstddef>

#include <glm.hpp>

#include "camera.hpp"
#include "component.hpp"

enum class CameraOutputTarget : int
{
    GameView = 0,
    EditorPreview = 1,
    RenderTexture = 2,
    Disabled = 3
};

/// Camera attached to a scene Object.
///
/// Position and orientation come from the owning object's world transform.
class CameraComponent final : public Component, public Camera
{
  public:
    ENGINE_API explicit CameraComponent(Object *parent);

    ENGINE_API glm::mat4 getViewMatrix() const override;

    ENGINE_API glm::mat4 getProjectionMatrix() const override;

    ENGINE_API CameraProjection getProjectionType() const override;

    ENGINE_API void setViewportSize(float width, float height) override;

    ENGINE_API void setProjectionType(CameraProjection projection);

    ENGINE_API void setOutputTarget(CameraOutputTarget output);

    ENGINE_API CameraOutputTarget getOutputTarget() const;

    ENGINE_API void setPriority(int value);
    ENGINE_API int getPriority() const;

    ENGINE_API void setClearColour(const glm::vec4 &colour);

    ENGINE_API const glm::vec4 &getClearColour() const;

  private:
    std::size_t GetPropertyCount() const override;

    bool GetProperty(std::size_t index, ComponentProperty &property) override;

    void OnPropertyChanged(const char *key) override;

    int projectionMode = static_cast<int>(CameraProjection::Perspective);

    int outputMode = static_cast<int>(CameraOutputTarget::GameView);

    float verticalFieldOfView = 60.0f;
    float orthographicSize = 5.0f;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;

    int priority = 0;

    glm::vec4 clearColour = {0.02f, 0.03f, 0.05f, 1.0f};

    glm::vec2 viewportSize = {1.0f, 1.0f};
};