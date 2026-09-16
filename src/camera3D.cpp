#include "camera3D.hpp"

#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>

Camera3D::Camera3D(float viewportWidth, float viewportHeight)
{
    setViewportSize(viewportWidth, viewportHeight);
}

glm::mat4 Camera3D::getViewMatrix() const
{
    return glm::lookAt(position, target, up);
}

glm::mat4 Camera3D::getProjectionMatrix() const
{
    return glm::perspective(
        glm::radians(verticalFovDegrees),
        aspectRatio,
        nearPlane,
        farPlane
    );
}

void Camera3D::setViewportSize(float width, float height)
{
    aspectRatio = std::max(width, 1.0f) / std::max(height, 1.0f);
}

void Camera3D::setPerspective(
    float newVerticalFovDegrees,
    float newNearPlane,
    float newFarPlane)
{
    verticalFovDegrees = newVerticalFovDegrees;
    nearPlane = newNearPlane;
    farPlane = newFarPlane;
}
