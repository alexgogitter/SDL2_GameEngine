#include "meshFilterComponent.hpp"

#include "resource_manager.hpp"

MeshFilterComponent::MeshFilterComponent(
    Object* parentObject,
    Resource_manager& resourceManager
)
    : Component("MeshFilter3D", parentObject),
      resources(&resourceManager)
{
    setBuiltInCube();
}

bool MeshFilterComponent::setMesh(
    MeshHandle newMesh
)
{
    if (
        resources == nullptr ||
        !resources->isMeshValid(newMesh)
    )
    {
        mesh = InvalidMeshHandle;
        refreshMetadata();
        return false;
    }

    mesh = newMesh;
    refreshMetadata();
    return true;
}

bool MeshFilterComponent::setBuiltInCube()
{
    if (resources == nullptr)
    {
        return false;
    }

    return setMesh(
        resources->loadBuiltInCubeMesh()
    );
}

void MeshFilterComponent::refreshMetadata()
{
    if (
        resources == nullptr ||
        !resources->isMeshValid(mesh)
    )
    {
        meshAssetPath = "None";
        vertexCount = 0;
        triangleCount = 0;
        return;
    }

    meshAssetPath =
        resources->getMeshAssetPath(mesh);

    vertexCount = static_cast<int>(
        resources->getMeshVertexCount(mesh)
    );

    triangleCount = static_cast<int>(
        resources->getMeshIndexCount(mesh) / 3
    );
}

std::size_t MeshFilterComponent::GetPropertyCount() const
{
    return 3;
}

bool MeshFilterComponent::GetProperty(
    std::size_t index,
    ComponentProperty& property
)
{
    switch (index)
    {
        case 0:
            property.key = "meshAsset";
            property.displayName = "Mesh";
            property.type =
                ComponentPropertyType::String;
            property.value = &meshAssetPath;
            property.readOnly = true;
            property.serializable = true;
            return true;

        case 1:
            property.key = "vertexCount";
            property.displayName = "Vertices";
            property.type =
                ComponentPropertyType::Integer;
            property.value = &vertexCount;
            property.readOnly = true;
            property.serializable = false;
            return true;

        case 2:
            property.key = "triangleCount";
            property.displayName = "Triangles";
            property.type =
                ComponentPropertyType::Integer;
            property.value = &triangleCount;
            property.readOnly = true;
            property.serializable = false;
            return true;

        default:
            return false;
    }
}