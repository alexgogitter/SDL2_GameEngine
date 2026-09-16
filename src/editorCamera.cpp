#include "editorCamera.hpp"

#include <algorithm>
#include <cmath>

#include <SDL.h>
#include <glm/geometric.hpp>

#include "eventListener.hpp"

EditorCamera::EditorCamera(float viewportWidth, float viewportHeight)
    : camera(viewportWidth, viewportHeight)
{
    camera.setPerspective(
        60.0f,
        0.05f,
        5000.0f
    );

    synchroniseCamera();
}

EditorCamera::~EditorCamera()
{
    if (looking)
    {
        SDL_SetRelativeMouseMode(SDL_FALSE);
    }
}

void EditorCamera::update(const EventListener& input, bool viewportHovered, float deltaSeconds)
{
    if (!looking && viewportHovered && input.WasMouseButtonPressed(SDL_BUTTON_RIGHT)
    )
    {
        looking = true;
        SDL_SetRelativeMouseMode(SDL_TRUE);
    }
    if (looking && !input.IsMouseButtonDown(SDL_BUTTON_RIGHT))
    {
        looking = false;
        SDL_SetRelativeMouseMode(SDL_FALSE);
    }

    if (!looking)
    {
        return;
    }

    const EventListener::MouseState& mouse = input.GetMouseState();

    yawDegrees += static_cast<float>(mouse.deltaX) * lookSensitivity;

    pitchDegrees -= static_cast<float>(mouse.deltaY) * lookSensitivity;

    pitchDegrees = std::clamp(pitchDegrees, -89.0f, 89.0f);

    const glm::vec3 forward = calculateForward();

    const glm::vec3 worldUp(0.0f, 1.0f, 0.0f);

    const glm::vec3 right = glm::normalize(glm::cross(forward, worldUp));

    glm::vec3 movement(0.0f);

    if (input.IsKeyDown(SDL_SCANCODE_W))
    {
        movement += forward;
    }

    if (input.IsKeyDown(SDL_SCANCODE_S))
    {
        movement -= forward;
    }

    if (input.IsKeyDown(SDL_SCANCODE_D))
    {
        movement += right;
    }

    if (input.IsKeyDown(SDL_SCANCODE_A))
    {
        movement -= right;
    }

    if (input.IsKeyDown(SDL_SCANCODE_E))
    {
        movement += worldUp;
    }

    if (input.IsKeyDown(SDL_SCANCODE_Q))
    {
        movement -= worldUp;
    }

    float currentSpeed = movementSpeed;

    if (input.IsKeyDown(SDL_SCANCODE_LSHIFT) || input.IsKeyDown(SDL_SCANCODE_RSHIFT))
    {
        currentSpeed *= 4.0f;
    }

    if (glm::dot(movement, movement) > 0.0f)
    {
        movement = glm::normalize(movement);

        position += movement * currentSpeed * std::clamp(deltaSeconds, 0.0f, 0.1f);
    }

    synchroniseCamera();
}

void EditorCamera::focus(const glm::vec3& worldPosition, float distance)
{
    distance = std::max(distance, 0.1f);

    position =
        worldPosition -
        calculateForward() * distance;

    synchroniseCamera();
}

Camera3D& EditorCamera::getCamera()
{
    return camera;
}

const glm::vec3&
EditorCamera::getPosition() const
{
    return position;
}

bool EditorCamera::isLooking() const
{
    return looking;
}

glm::vec3 EditorCamera::calculateForward() const
{
    const float yaw =
        glm::radians(yawDegrees);

    const float pitch =
        glm::radians(pitchDegrees);

    return glm::normalize(glm::vec3(
        std::cos(yaw) * std::cos(pitch),
        std::sin(pitch),
        std::sin(yaw) * std::cos(pitch)
    ));
}

void EditorCamera::synchroniseCamera()
{
    camera.setPosition(position);

    camera.lookAt(position + calculateForward());
}