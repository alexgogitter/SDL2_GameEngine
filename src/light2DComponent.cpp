#include "light2DComponent.hpp"

#include "object.hpp"
#include "render2D.hpp"

Light2DComponent::Light2DComponent(
    Object* parentObject,
    const glm::vec3& colour,
    float intensity,
    float radius,
    float height)
    : Component("PointLight2D", parentObject)
{
    setColour(colour);
    setIntensity(intensity);
    setRadius(radius);
    setHeight(height);
}

void Light2DComponent::Update(std::uint64_t)
{
}

std::size_t Light2DComponent::GetPropertyCount() const
{
    return 5;
}

bool Light2DComponent::GetProperty(
    std::size_t index,
    ComponentProperty& property)
{
    switch (index)
    {
        case 0:
            property.key = "colour";
            property.displayName = "Colour";
            property.type =
                ComponentPropertyType::Colour3;
            property.value = &light.colour;
            return true;

        case 1:
            property.key = "intensity";
            property.displayName = "Intensity";
            property.type =
                ComponentPropertyType::Float;
            property.value = &light.intensity;
            property.hasRange = true;
            property.minimum = 0.0f;
            property.maximum = 25.0f;
            property.step = 0.05f;
            return true;

        case 2:
            property.key = "radius";
            property.displayName = "Radius";
            property.type =
                ComponentPropertyType::Float;
            property.value = &light.radius;
            property.hasRange = true;
            property.minimum = 1.0f;
            property.maximum = 5000.0f;
            property.step = 1.0f;
            return true;

        case 3:
            property.key = "height";
            property.displayName = "Height";
            property.type =
                ComponentPropertyType::Float;
            property.value = &light.position.z;
            property.hasRange = true;
            property.minimum = 0.0f;
            property.maximum = 2000.0f;
            property.step = 1.0f;
            return true;

        case 4:
            property.key = "castsShadows";
            property.displayName = "Cast Shadows";
            property.type =
                ComponentPropertyType::Boolean;
            property.value = &light.castsShadows;
            return true;
    }

    return false;
}

void Light2DComponent::OnPropertyChanged(const char*)
{
    setIntensity(light.intensity);
    setRadius(light.radius);
    setHeight(light.position.z);
}

void Light2DComponent::Draw2D(Renderer2D* renderer)
{
    if (renderer == nullptr || parent == nullptr)
    {
        return;
    }

    PointLight2D submittedLight = light;
    submittedLight.position.x = parent->getWorldPosition().x;
    submittedLight.position.y = parent->getWorldPosition().y;
    renderer->SubmitLight2D(
        submittedLight,
        parent->getLayer()
    );
}
