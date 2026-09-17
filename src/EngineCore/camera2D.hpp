#pragma once
#include "camera.hpp"
#include "engineApi.hpp"

/// Orthographic camera preserving the engine's pixel-space, positive-Y-down world.
class Camera2D final : public Camera
{
  public:
    /// Creates a camera centred on the supplied logical viewport.
    /// @param viewportWidth Logical viewport width in pixels.
    /// @param viewportHeight Logical viewport height in pixels.
    ENGINE_API Camera2D(float viewportWidth = 1.0f, float viewportHeight = 1.0f);

    /// @return Inverse camera transform for 2D position and rotation.
    ENGINE_API glm::mat4 getViewMatrix() const override;
    /// @return Positive-Y-down orthographic projection using the current zoom.
    ENGINE_API glm::mat4 getProjectionMatrix() const override;
    /// @return CameraProjection::Orthographic.
    CameraProjection getProjectionType() const override { return CameraProjection::Orthographic; }

    /// Updates logical viewport dimensions, clamping each dimension to at least one.
    ENGINE_API void setViewportSize(float width, float height) override;
    /// Sets the world-space centre of the camera in pixels.
    void setPosition(const glm::vec2 &value) { position = value; }
    /// Sets camera rotation in radians.
    void setRotation(float radians) { rotation = radians; }
    /// Sets magnification, clamped to at least 0.01. Values above one zoom in.
    ENGINE_API void setZoom(float value);

    /// @return World-space camera centre in pixels.
    const glm::vec2 &getPosition() const { return position; }
    /// @return Camera rotation in radians.
    float getRotation() const { return rotation; }
    /// @return Current positive magnification value.
    float getZoom() const { return zoom; }
    /// @return Logical viewport dimensions in pixels.
    const glm::vec2 &getViewportSize() const { return viewportSize; }

    /// Converts SDL window coordinates into world coordinates.
    /// @param screenPosition Logical pixel coordinates relative to the window.
    /// @return World-space position accounting for camera position, rotation, and zoom.
    ENGINE_API glm::vec2 screenToWorld(const glm::vec2 &screenPosition) const;

  private:
    glm::vec2 position = {0.5f, 0.5f};
    glm::vec2 viewportSize = {1.0f, 1.0f};
    float rotation = 0.0f;
    float zoom = 1.0f;
};
