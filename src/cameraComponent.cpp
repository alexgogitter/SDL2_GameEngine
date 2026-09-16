#include "cameraComponent.hpp"

#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>

#include "object.hpp"

CameraComponent::CameraComponent(
    Object* parentObject
)
    : Component("Camera", parentObject)
{
}

glm::mat4 CameraComponent::getViewMatrix() const
{
    if (parent == nullptr)
    {
        return glm::mat4(1.0f);
    }

    const glm::vec3 position =
        parent->getWorldPosition();

    const glm::quat rotation =
        parent->getWorldRotation();

    const glm::vec3 forward =
        rotation *
        glm::vec3(0.0f, 0.0f, -1.0f);

    const glm::vec3 up =
        rotation *
        glm::vec3(0.0f, 1.0f, 0.0f);

    return glm::lookAt(
        position,
        position + forward,
        up
    );
}

glm::mat4 CameraComponent::getProjectionMatrix() const
{
    const float width =
        std::max(viewportSize.x, 1.0f);

    const float height =
        std::max(viewportSize.y, 1.0f);

    const float aspectRatio =
        width / height;

    if (
        getProjectionType() ==
        CameraProjection::Orthographic
    )
    {
        const float halfHeight =
            orthographicSize;

        const float halfWidth =
            halfHeight * aspectRatio;

        return glm::ortho(
            -halfWidth,
            halfWidth,
            -halfHeight,
            halfHeight,
            nearPlane,
            farPlane
        );
    }

    return glm::perspective(
        glm::radians(verticalFieldOfView),
        aspectRatio,
        nearPlane,
        farPlane
    );
}

CameraProjection
CameraComponent::getProjectionType() const
{
    return projectionMode ==
        static_cast<int>(
            CameraProjection::Orthographic
        )
            ? CameraProjection::Orthographic
            : CameraProjection::Perspective;
}

void CameraComponent::setViewportSize(
    float width,
    float height
)
{
    viewportSize = {
        std::max(width, 1.0f),
        std::max(height, 1.0f)
    };
}

void CameraComponent::setProjectionType(
    CameraProjection projection
)
{
    projectionMode =
        static_cast<int>(projection);
}

void CameraComponent::setOutputTarget(
    CameraOutputTarget output
)
{
    outputMode = static_cast<int>(output);
}

CameraOutputTarget
CameraComponent::getOutputTarget() const
{
    return static_cast<CameraOutputTarget>(
        outputMode
    );
}

void CameraComponent::setPriority(int value)
{
    priority = value;
}

int CameraComponent::getPriority() const
{
    return priority;
}

void CameraComponent::setClearColour(
    const glm::vec4& colour
)
{
    clearColour = colour;
}

const glm::vec4&
CameraComponent::getClearColour() const
{
    return clearColour;
}

std::size_t
CameraComponent::GetPropertyCount() const
{
    return 8;
}

bool CameraComponent::GetProperty(
    std::size_t index,
    ComponentProperty& property
)
{
    static const char* ProjectionLabels[] = {
        "Orthographic",
        "Perspective"
    };

    static const char* OutputLabels[] = {
        "Game View",
        "Editor Preview",
        "Render Texture",
        "Disabled"
    };

    switch (index)
    {
        case 0:
            property.key = "projection";
            property.displayName = "Projection";
            property.type =
                ComponentPropertyType::Enumeration;
            property.value = &projectionMode;
            property.enumerationLabels =
                ProjectionLabels;
            property.enumerationCount = 2;
            return true;

        case 1:
            property.key = "fieldOfView";
            property.displayName =
                "Field of View";
            property.type =
                ComponentPropertyType::Float;
            property.value =
                &verticalFieldOfView;
            property.hasRange = true;
            property.minimum = 1.0f;
            property.maximum = 179.0f;
            property.step = 0.1f;
            return true;

        case 2:
            property.key = "orthographicSize";
            property.displayName =
                "Orthographic Size";
            property.type =
                ComponentPropertyType::Float;
            property.value =
                &orthographicSize;
            property.hasRange = true;
            property.minimum = 0.01f;
            property.maximum = 10000.0f;
            property.step = 0.1f;
            return true;

        case 3:
            property.key = "nearPlane";
            property.displayName =
                "Near Clip";
            property.type =
                ComponentPropertyType::Float;
            property.value = &nearPlane;
            property.hasRange = true;
            property.minimum = 0.001f;
            property.maximum = 100.0f;
            property.step = 0.01f;
            return true;

        case 4:
            property.key = "farPlane";
            property.displayName =
                "Far Clip";
            property.type =
                ComponentPropertyType::Float;
            property.value = &farPlane;
            property.hasRange = true;
            property.minimum = 0.01f;
            property.maximum = 100000.0f;
            property.step = 1.0f;
            return true;

        case 5:
            property.key = "output";
            property.displayName = "Output";
            property.type =
                ComponentPropertyType::Enumeration;
            property.value = &outputMode;
            property.enumerationLabels =
                OutputLabels;
            property.enumerationCount = 4;
            return true;

        case 6:
            property.key = "priority";
            property.displayName = "Priority";
            property.type =
                ComponentPropertyType::Integer;
            property.value = &priority;
            property.step = 1.0f;
            return true;

        case 7:
            property.key = "clearColour";
            property.displayName =
                "Clear Colour";
            property.type =
                ComponentPropertyType::Colour4;
            property.value = &clearColour;
            return true;

        default:
            return false;
    }
}

void CameraComponent::OnPropertyChanged(
    const char*
)
{
    projectionMode = std::clamp(
        projectionMode,
        static_cast<int>(
            CameraProjection::Orthographic
        ),
        static_cast<int>(
            CameraProjection::Perspective
        )
    );

    outputMode = std::clamp(
        outputMode,
        static_cast<int>(
            CameraOutputTarget::GameView
        ),
        static_cast<int>(
            CameraOutputTarget::Disabled
        )
    );

    verticalFieldOfView = std::clamp(
        verticalFieldOfView,
        1.0f,
        179.0f
    );

    orthographicSize =
        std::max(orthographicSize, 0.01f);

    nearPlane =
        std::max(nearPlane, 0.001f);

    farPlane =
        std::max(farPlane, nearPlane + 0.001f);
}