#pragma once

#include <cstddef>
#include <string>

#include "component.hpp"
#include "mesh3D.hpp"

class Resource_manager;

/// References geometry without deciding how it is rendered.
class MeshFilterComponent final : public Component
{
  public:
    MeshFilterComponent(Object *parent, Resource_manager &resources);

    bool setMesh(MeshHandle mesh);
    bool setBuiltInCube();

    MeshHandle getMesh() const { return mesh; }

  private:
    std::size_t GetPropertyCount() const override;

    bool GetProperty(std::size_t index, ComponentProperty &property) override;

    void refreshMetadata();

    Resource_manager *resources = nullptr;
    MeshHandle mesh = InvalidMeshHandle;

    std::string meshAssetPath = "None";
    int vertexCount = 0;
    int triangleCount = 0;
};