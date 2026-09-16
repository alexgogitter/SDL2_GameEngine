#pragma once

#include <cstddef>

#include <glm/glm.hpp>

#include "component.hpp"

class Resource_manager;
class Renderer3D;

class MeshRendererComponent final
    : public Component
{
public:
    ENGINE_API MeshRendererComponent(
        Object* parent,
        Resource_manager& resources
    );

private:
    void Draw3D(Renderer3D* renderer) override;

    std::size_t GetPropertyCount()
        const override;

    bool GetProperty(
        std::size_t index,
        ComponentProperty& property
    ) override;

    Resource_manager* resources = nullptr;

    glm::vec4 colour = {
        0.18f,
        0.55f,
        0.95f,
        1.0f
    };
};