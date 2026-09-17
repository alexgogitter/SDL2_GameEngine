#pragma once

#include "component.hpp"
#include <cstddef>
#include <cstdint>
#include <glm.hpp>
#include <memory>

class PhysicsWorld3D;
class BoxCollider3DComponent;

class Rigidbody3DComponent final : public Component
{
  public:
    ENGINE_API Rigidbody3DComponent(Object *parent, PhysicsWorld3D &world);

    ENGINE_API ~Rigidbody3DComponent() override;

    Rigidbody3DComponent(const Rigidbody3DComponent &) = delete;

    Rigidbody3DComponent &operator=(const Rigidbody3DComponent &) = delete;

    ENGINE_API bool isValid() const;

  private:
    friend class BoxCollider3DComponent;

    struct Impl;

    void Update(std::uint64_t deltaTime) override;

    std::size_t GetPropertyCount() const override;

    bool GetProperty(std::size_t index, ComponentProperty &property) override;

    void OnPropertyChanged(const char *key) override;

    void OnTransformChanged() override;

    void *getActorHandle() const;

    void OnEnable() override;
    void OnDisable() override;

    std::unique_ptr<Impl> impl;

    float mass = 1.0f;
    bool useGravity = true;
    bool isKinematic = false;
    glm::vec3 linearVelocity = {0.0f, 0.0f, 0.0f};

    glm::vec3 measuredAcceleration = {0.0f, 0.0f, 0.0f};

    glm::vec3 previousLinearVelocity = {0.0f, 0.0f, 0.0f};

    glm::vec3 physicsRotationDegrees = {0.0f, 0.0f, 0.0f};

    glm::vec3 angularVelocityDegrees = {0.0f, 0.0f, 0.0f};

    float linearDamping = 0.0f;
    float angularDamping = 0.0f;

    bool hasPreviousLinearVelocity = false;

    PhysicsWorld3D *physicsWorld = nullptr;
};