#ifndef OBJECT_H
#define OBJECT_H

#include <cstdint>
#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include <SDL.h>
#include <SDL_image.h>

#include "component.hpp"
#include "glm/glm.hpp"
#include "render2D.hpp"
#include "render3D.hpp"
#include "resource_manager.hpp"
#include "transform3D.hpp"

#include "engineApi.hpp"
#include "objectId.hpp"

class Scene;

/// Owning entity container for a transform, components, and child objects.
class Object
{
private:
    friend class Scene;

    ObjectId id = InvalidObjectId;
    Object* parentObject = nullptr;
    std::string name;
    bool active = true;
    int layer = 0;
    std::vector<Object*> child_Objects;
    std::map<std::string, Component*> components;
    SDL_Rect entity_box = {-1, -1, 2, 2};
    Renderer2D* renderer = nullptr;
    Renderer3D* renderer3D = nullptr;

    Object* detachChild(Object* object);

public:
    Transform3D transform;
    glm::vec4 draw_colour = glm::vec4(255.0f, 0.0f, 0.0f, 255.0f);

    /// Creates a named object associated with the current engine services.
    /// @param obj_name Stable debug/entity name copied by the object.
    /// @param resourceManager Present for current object-construction compatibility.
    /// @param rend Non-owned renderer pointer.
    ENGINE_API Object(
    const std::string& objectName,
    Resource_manager& resourceManager,
    Renderer2D* renderer,
    Renderer3D* renderer3D,
    ObjectId requestedId = InvalidObjectId
    );

    /// Deletes all owned components and child objects.
    ENGINE_API virtual ~Object();

    /// Updates the object, every component, and every child.
    /// @param deltaTime Elapsed frame time in milliseconds.
    ENGINE_API void update(std::uint64_t deltaTime);

    /// Object-specific update hook implemented by concrete entity types.
    virtual void Update(std::uint64_t) {}

    /// Submits fallback colour, component, debug collider, and child draw commands.
    /// Sprite components use their own draw rects; collider debug uses collider size.
    ENGINE_API void draw2D(Renderer2D* renderer);
    ENGINE_API void draw3D(Renderer3D* renderer);

    /// Transfers ownership of a component to this object.
    /// A component with the same name is deleted and replaced.
    /// @return The transferred pointer, or nullptr when component is null.
    ENGINE_API Component* add_Component(Component* component);

    template<typename T>
    T* getComponent()
    {
        static_assert(
            std::is_base_of_v<Component, T>,
            "T must derive from Component"
        );

        for (auto& entry : components)
        {
            if (auto* typedComponent = dynamic_cast<T*>(entry.second))
                return typedComponent;
        }

        return nullptr;
    }

    ENGINE_API std::size_t getComponentCount() const;

    /// Returns nullptr when index is outside the component collection.
    ENGINE_API Component* getComponentAt(std::size_t index) const;

    /// Returns nullptr when the type is not attached.
    ENGINE_API Component* getComponentByTypeName(
        const std::string& typeName
    ) const;

    /// Deletes and removes the supplied component by its name.
    /// @return Zero on success, or -1 when component is null/not found.
    ENGINE_API int remove_Component(Component* component);

    /// Deletes and removes a component by name.
    /// @return Zero on success, or -1 when not found.
    ENGINE_API int remove_Component(const std::string& componentName);

    ENGINE_API void notifyTransformChanged();

    /// Transfers ownership of a child object.
    /// @return Zero on success, or -1 when object is null.
    ENGINE_API int add_Child_Object(Object* object);

    /// Deletes and removes an owned child.
    /// @return Zero on success, or -1 when not found.
    ENGINE_API int remove_Child_Object(Object* object);

    ENGINE_API Object* getParentObject() const;
    /// Returns the composed local-to-world transformation.
    ENGINE_API glm::mat4 getWorldMatrix() const;

    /// Returns a decomposed copy of the world transform.
    ENGINE_API Transform3D getWorldTransform() const;

    ENGINE_API glm::vec3 getWorldPosition() const;
    ENGINE_API glm::quat getWorldRotation() const;
    
    ENGINE_API std::size_t getChildCount() const;
    ENGINE_API Object* getChild(std::size_t index) const;

    /// Searches this object and all descendants.
    ENGINE_API Object* findInHierarchy(ObjectId objectId);

    ENGINE_API ObjectId getId() const;

    ENGINE_API const std::string& getName() const;
    ENGINE_API void setName(const std::string& newName);

    ENGINE_API bool isActive() const;
    ENGINE_API void setActive(bool value);

    ENGINE_API int getLayer() const;

    /// Returns false when newLayer is outside the valid 0–31 range.
    ENGINE_API bool setLayer(int newLayer);

    Object(const Object&) = delete;
    Object& operator=(const Object&) = delete;
    Object(Object&&) = delete;
    Object& operator=(Object&&) = delete;
};

#endif // OBJECT_H
