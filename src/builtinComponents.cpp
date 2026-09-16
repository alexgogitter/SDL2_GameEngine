#include "builtInComponents.hpp"

#include "animationControllerComponent.hpp"
#include "colliderComponent.hpp"
#include "componentRegistry.hpp"
#include "light2DComponent.hpp"
#include "object.hpp"
#include "rigidbodyComponent.hpp"
#include "spriteRendererComponent.hpp"
#include "meshFilterComponent.hpp"
#include "cameraComponent.hpp"
#include "meshRendererComponent.hpp"
#include "rigidbody3DComponent.hpp"
#include "boxCollider3DComponent.hpp"

#include <algorithm>

namespace
{

    constexpr const char* const
    BoxCollider2DRequirements[] =
    {
        "Rigidbody2D"
    };

    constexpr const char* const
        AnimationControllerRequirements[] =
    {
        "SpriteRenderer2D"
    };

    constexpr const char* const
        MeshRendererRequirements[] =
    {
        "MeshFilter3D"
    };

    Component* CreateSpriteRenderer(
        Object* parent,
        const ComponentCreateContext& context)
    {
        if (parent == nullptr || context.resources == nullptr)
        {
            return nullptr;
        }

        return new SpriteRendererComponent(
            parent,
            *context.resources
        );
    }

    Component* CreateCamera(
        Object* parent,
        const ComponentCreateContext&
    )
    {
        return parent != nullptr
            ? new CameraComponent(parent)
            : nullptr;
    }

    Component* CreatePointLight(
        Object* parent,
        const ComponentCreateContext&)
    {
        return parent != nullptr
            ? new Light2DComponent(parent)
            : nullptr;
    }

    Component* CreateRigidbody(
        Object* parent,
        const ComponentCreateContext& context)
    {
        if (parent == nullptr || context.physicsWorld == nullptr)
        {
            return nullptr;
        }

        return new RigidbodyComponent(parent, *context.physicsWorld, BodyType2D::Dynamic);
    }

    Component* CreateRigidbody3D(Object* parent, const ComponentCreateContext& context)
    {
        if (parent == nullptr || context.physicsWorld3D == nullptr)
        {
            return nullptr;
        }

        return new Rigidbody3DComponent(parent, *context.physicsWorld3D);
    }

    Component* CreateBoxCollider(Object* parent, const ComponentCreateContext& context)
    {
        if (parent == nullptr)
        {
            return nullptr;
        }

        auto* rigidbody =
            parent->getComponent<RigidbodyComponent>();

        if (rigidbody == nullptr)
        {
            return nullptr;
        }

        glm::vec2 sizePixels =
            parent->transform.getScale2D() * 2.0f;

        sizePixels.x = std::max(sizePixels.x, 1.0f);
        sizePixels.y = std::max(sizePixels.y, 1.0f);

        return new ColliderComponent(
            parent,
            *rigidbody,
            sizePixels
        );
    }

    Component* CreateBoxCollider3D(
    Object* parent,
    const ComponentCreateContext& context
    )
    {
        if (
            parent == nullptr ||
            context.physicsWorld3D == nullptr
        )
        {
            return nullptr;
        }

        auto* rigidbody =
            parent->getComponent<
                Rigidbody3DComponent
            >();

        auto* collider =
            new BoxCollider3DComponent(
                parent,
                *context.physicsWorld3D,
                rigidbody
            );

        if (!collider->isValid())
        {
            delete collider;
            return nullptr;
        }

        return collider;
    }

    Component* CreateAnimationController(
        Object* parent,
        const ComponentCreateContext& context)
    {
        if (
            parent == nullptr ||
            context.resources == nullptr
        )
        {
            return nullptr;
        }

        auto* sprite =
            parent->getComponent<SpriteRendererComponent>();

        if (sprite == nullptr)
        {
            return nullptr;
        }

        return new AnimationControllerComponent(
            parent,
            *context.resources,
            *sprite
        );
    }

    Component* CreateMeshRenderer(
        Object* parent,
        const ComponentCreateContext& context
    )
    {
        if (
            parent == nullptr ||
            context.resources == nullptr
        )
        {
            return nullptr;
        }

        return new MeshRendererComponent(
            parent,
            *context.resources
        );
    }

    Component* CreateMeshFilter(
        Object* parent,
        const ComponentCreateContext& context
    )
    {
        if (
            parent == nullptr ||
            context.resources == nullptr
        )
        {
            return nullptr;
        }

        return new MeshFilterComponent(
            parent,
            *context.resources
        );
    }

    bool RegisterIfMissing(
        ComponentRegistry& registry,
        const char* typeName,
        const char* displayName,
        const char* category,
        ComponentFactory factory,
        const char* const* requiredTypes,
        std::size_t requiredTypeCount
    )
    {
        return registry.contains(typeName) ||
            registry.registerType(
                typeName,
                displayName,
                category,
                factory,
                requiredTypes,
                requiredTypeCount
            );
    }
}

bool RegisterBuiltInComponents(ComponentRegistry& registry)
{
    bool success = true;

    success = RegisterIfMissing(
        registry,
        "SpriteRenderer2D",
        "Sprite Renderer 2D",
        "Rendering",
        &CreateSpriteRenderer,
        nullptr,
        0
    ) && success;

    success = RegisterIfMissing(
        registry,
        "PointLight2D",
        "Point Light 2D",
        "Lighting",
        &CreatePointLight,
        nullptr,
        0
    ) && success;

    success = RegisterIfMissing(
        registry,
        "Rigidbody2D",
        "Rigidbody 2D",
        "Physics",
        &CreateRigidbody,
        nullptr,
        0
    ) && success;

    success = RegisterIfMissing(
        registry,
        "Rigidbody3D",
        "Rigidbody 3D",
        "Physics",
        &CreateRigidbody3D,
        nullptr,
        0
    ) && success;

    success = RegisterIfMissing(
        registry,
        "BoxCollider2D",
        "Box Collider 2D",
        "Physics",
        &CreateBoxCollider,
        BoxCollider2DRequirements,
        1
    ) && success;

    success = RegisterIfMissing(
        registry,
        "BoxCollider3D",
        "Box Collider 3D",
        "Physics",
        &CreateBoxCollider3D,
        nullptr,
        0
    ) && success;

    success = RegisterIfMissing(
        registry,
        "AnimationController2D",
        "Animation Controller 2D",
        "Animation",
        &CreateAnimationController,
        AnimationControllerRequirements,
        1
    ) && success;
    
    success = RegisterIfMissing(
        registry,
        "MeshFilter3D",
        "Mesh Filter 3D",
        "Rendering",
        &CreateMeshFilter,
        nullptr,
        0
    ) && success;

    success = RegisterIfMissing(
        registry,
        "MeshRenderer3D",
        "Mesh Renderer 3D",
        "Rendering",
        &CreateMeshRenderer,
        MeshRendererRequirements,
        1
    ) && success;

    success = RegisterIfMissing(
        registry,
        "Camera",
        "Camera",
        "Rendering",
        &CreateCamera,
        nullptr,
        0
    ) && success;

    return success;
}