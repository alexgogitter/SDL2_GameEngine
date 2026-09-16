#pragma once
#include "engineApi.hpp"
#include "camera.hpp"

/// Perspective camera foundation for the future 3D mesh render pass.
/// The camera matrices work now; Render2D does not render 3D meshes.
class Camera3D final : public Camera
{
public:
    /// Creates a perspective camera with a viewport-derived aspect ratio.
    ENGINE_API Camera3D(float viewportWidth = 1.0f, float viewportHeight = 1.0f);

    /// @return Right-handed look-at view matrix.
    ENGINE_API glm::mat4 getViewMatrix() const override;
    /// @return Perspective projection matrix using the configured clip planes.
    ENGINE_API glm::mat4 getProjectionMatrix() const override;
    /// @return CameraProjection::Perspective.
    CameraProjection getProjectionType() const override
    {
        return CameraProjection::Perspective;
    }

    /// Updates the projection aspect ratio from logical viewport dimensions.
    ENGINE_API void setViewportSize(float width, float height) override;
    /// Sets world-space eye position.
    void setPosition(const glm::vec3& value) { position = value; }
    /// Sets the world-space point observed by the camera.
    void lookAt(const glm::vec3& value) { target = value; }
    /// Configures perspective projection.
    /// @param verticalFovDegrees Vertical field of view in degrees.
    /// @param nearPlane Positive near clipping distance.
    /// @param farPlane Far clipping distance greater than nearPlane.
    ENGINE_API void setPerspective(float verticalFovDegrees, float nearPlane, float farPlane);

    /// @return World-space eye position.
    const glm::vec3& getPosition() const { return position; }

private:
    glm::vec3 position = {0.0f, 0.0f, 5.0f};
    glm::vec3 target = {0.0f, 0.0f, 0.0f};
    glm::vec3 up = {0.0f, 1.0f, 0.0f};
    float aspectRatio = 1.0f;
    float verticalFovDegrees = 60.0f;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;
};
