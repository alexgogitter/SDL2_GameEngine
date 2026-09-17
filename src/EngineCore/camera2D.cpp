#include "camera2D.hpp"

#include <algorithm>

#include <gtc/matrix_inverse.hpp>
#include <gtc/matrix_transform.hpp>

Camera2D::Camera2D(float viewportWidth, float viewportHeight)
{
    setViewportSize(viewportWidth, viewportHeight);
    position = viewportSize * 0.5f;
}

glm::mat4 Camera2D::getViewMatrix() const
{
    glm::mat4 transform(1.0f);
    transform = glm::translate(transform, glm::vec3(position, 0.0f));
    transform = glm::rotate(transform, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    return glm::inverse(transform);
}

glm::mat4 Camera2D::getProjectionMatrix() const
{
    const float halfWidth = viewportSize.x * 0.5f / zoom;
    const float halfHeight = viewportSize.y * 0.5f / zoom;

    // Reversed top/bottom keeps the engine's existing screen-space +Y-down
    // convention while still using a conventional OpenGL camera.
    return glm::ortho(-halfWidth, halfWidth, halfHeight, -halfHeight, -100.0f, 100.0f);
}

void Camera2D::setViewportSize(float width, float height)
{
    viewportSize.x = std::max(width, 1.0f);
    viewportSize.y = std::max(height, 1.0f);
}

void Camera2D::setZoom(float value) { zoom = std::max(value, 0.01f); }

glm::vec2 Camera2D::screenToWorld(const glm::vec2 &screenPosition) const
{
    const glm::vec2 normalisedDevicePosition = {(2.0f * screenPosition.x / viewportSize.x) - 1.0f, 1.0f - (2.0f * screenPosition.y / viewportSize.y)};

    const glm::vec4 worldPosition = glm::inverse(getViewProjectionMatrix()) * glm::vec4(normalisedDevicePosition, 0.0f, 1.0f);

    return glm::vec2(worldPosition) / worldPosition.w;
}
