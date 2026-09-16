#pragma once

#include <glm/glm.hpp>

#include "camera3D.hpp"
#include "engineApi.hpp"

class EventListener;

class EditorCamera final
{
public:
    ENGINE_API EditorCamera(
        float viewportWidth = 960.0f,
        float viewportHeight = 540.0f
    );

    ENGINE_API ~EditorCamera();

    ENGINE_API void update(
        const EventListener& input,
        bool viewportHovered,
        float deltaSeconds
    );

    ENGINE_API void focus(
        const glm::vec3& worldPosition,
        float distance = 10.0f
    );

    ENGINE_API Camera3D& getCamera();
    ENGINE_API const glm::vec3& getPosition() const;
    ENGINE_API bool isLooking() const;

private:
    glm::vec3 calculateForward() const;
    void synchroniseCamera();

    Camera3D camera;
    
    glm::vec3 position =
        {0.0f, 4.0f, 10.0f};

    float yawDegrees = -90.0f;
    float pitchDegrees = -22.0f;

    float movementSpeed = 8.0f;
    float lookSensitivity = 0.12f;

    bool looking = false;
};