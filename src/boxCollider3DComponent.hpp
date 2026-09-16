#pragma once

#include <cstddef>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "component.hpp"
#include "physicsWorld3D.hpp"

class Rigidbody3DComponent;
class PhysicsWorld3D;

class BoxCollider3DComponent final
    : public Component
{
public:
    ENGINE_API BoxCollider3DComponent(
        Object* parent,
        PhysicsWorld3D& world,
        Rigidbody3DComponent* rigidbody
    );

    ENGINE_API ~BoxCollider3DComponent()
        override;

    BoxCollider3DComponent(
        const BoxCollider3DComponent&
    ) = delete;

    BoxCollider3DComponent& operator=(
        const BoxCollider3DComponent&
    ) = delete;

    ENGINE_API bool isValid() const;

private:
    struct Impl;

    std::size_t GetPropertyCount()
        const override;

    bool GetProperty(
        std::size_t index,
        ComponentProperty& property
    ) override;

    void OnPropertyChanged(
        const char* key
    ) override;

    void OnTransformChanged() override;

    void updateGeometry();
    void updateMaterial();
    void updateShapeFlags();
    void updateMassProperties();

    void OnEnable() override;
    void OnDisable() override;

    std::unique_ptr<Impl> impl;

    glm::vec3 size =
        {1.0f, 1.0f, 1.0f};

    glm::vec3 center =
        {0.0f, 0.0f, 0.0f};

    float staticFriction = 0.6f;
    float dynamicFriction = 0.6f;
    float restitution = 0.0f;

    bool isTrigger = false;
};