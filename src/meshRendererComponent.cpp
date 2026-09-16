#include "meshRendererComponent.hpp"

#include "meshFilterComponent.hpp"
#include "object.hpp"
#include "render3D.hpp"
#include "resource_manager.hpp"

MeshRendererComponent::
MeshRendererComponent(
    Object* parentObject,
    Resource_manager& resourceManager
)
    : Component(
        "MeshRenderer3D",
        parentObject
    ),
      resources(&resourceManager)
{
}

void MeshRendererComponent::Draw3D(
    Renderer3D* renderer
)
{
    if (
        renderer == nullptr ||
        parent == nullptr ||
        resources == nullptr
    )
    {
        return;
    }

    MeshFilterComponent* filter =
        parent->getComponent<
            MeshFilterComponent
        >();

    if (filter == nullptr)
    {
        return;
    }

    MeshRenderState mesh;

    if (
        !resources->getMeshRenderState(
            filter->getMesh(),
            mesh
        )
    )
    {
        return;
    }

    renderer->SubmitMesh(
        mesh,
        parent->getWorldMatrix(),
        colour,
        parent->getLayer()
    );
}

std::size_t
MeshRendererComponent::GetPropertyCount()
    const
{
    return 1;
}

bool MeshRendererComponent::GetProperty(
    std::size_t index,
    ComponentProperty& property
)
{
    if (index != 0)
    {
        return false;
    }

    property.key = "colour";
    property.displayName = "Colour";
    property.type =
        ComponentPropertyType::Colour4;
    property.value = &colour;

    return true;
}