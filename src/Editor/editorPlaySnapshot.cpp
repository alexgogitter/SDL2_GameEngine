#include "editorPlaySnapshot.hpp"

#include <cstring>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include <glm.hpp>

#include "component.hpp"
#include "componentProperty.hpp"
#include "object.hpp"
#include "objectId.hpp"
#include "scene.hpp"
#include "transform3D.hpp"

namespace
{
using SnapshotValue = std::variant<bool, int, float, glm::vec2, glm::vec3, glm::vec4, std::string>;

struct PropertySnapshot
{
    std::string key;
    ComponentPropertyType type = ComponentPropertyType::Float;

    SnapshotValue value = false;
};

struct ComponentSnapshot
{
    std::string typeName;
    bool enabled = true;

    std::vector<PropertySnapshot> properties;
};

struct ObjectSnapshot
{
    ObjectId id = InvalidObjectId;
    std::string name;
    bool active = true;
    int layer = 0;
    Transform3D transform;

    std::vector<ComponentSnapshot> components;
};

bool CaptureProperty(const ComponentProperty &property, PropertySnapshot &snapshot)
{
    if (property.value == nullptr || !property.serializable || property.readOnly) {
        return false;
    }

    snapshot.key = property.key;
    snapshot.type = property.type;

    switch (property.type) {
    case ComponentPropertyType::Boolean:
        snapshot.value = *static_cast<const bool *>(property.value);
        return true;

    case ComponentPropertyType::Integer:
    case ComponentPropertyType::Enumeration:
        snapshot.value = *static_cast<const int *>(property.value);
        return true;

    case ComponentPropertyType::Float:
        snapshot.value = *static_cast<const float *>(property.value);
        return true;

    case ComponentPropertyType::Vector2:
        snapshot.value = *static_cast<const glm::vec2 *>(property.value);
        return true;

    case ComponentPropertyType::Vector3:
    case ComponentPropertyType::Colour3:
        snapshot.value = *static_cast<const glm::vec3 *>(property.value);
        return true;

    case ComponentPropertyType::Vector4:
    case ComponentPropertyType::Colour4:
        snapshot.value = *static_cast<const glm::vec4 *>(property.value);
        return true;

    case ComponentPropertyType::String:
        snapshot.value = *static_cast<const std::string *>(property.value);
        return true;
    }

    return false;
}

ComponentSnapshot CaptureComponent(Component &component)
{
    ComponentSnapshot snapshot;

    snapshot.typeName = component.getTypeName();

    snapshot.enabled = component.isEnabled();

    for (std::size_t index = 0; index < component.getPropertyCount(); ++index) {
        ComponentProperty property;

        if (!component.getProperty(index, property)) {
            continue;
        }

        PropertySnapshot propertySnapshot;

        if (CaptureProperty(property, propertySnapshot)) {
            snapshot.properties.push_back(std::move(propertySnapshot));
        }
    }

    return snapshot;
}

void ApplyPropertyValue(ComponentProperty &property, const SnapshotValue &value)
{
    std::visit(
        [&property](const auto &storedValue) {
            using ValueType = std::decay_t<decltype(storedValue)>;

            *static_cast<ValueType *>(property.value) = storedValue;
        },
        value);
}

void RestoreComponent(Object &object, const ComponentSnapshot &snapshot)
{
    Component *component = object.getComponentByTypeName(snapshot.typeName);

    if (component == nullptr) {
        return;
    }

    component->setEnabled(snapshot.enabled);

    for (const PropertySnapshot &propertySnapshot : snapshot.properties) {
        for (std::size_t index = 0; index < component->getPropertyCount(); ++index) {
            ComponentProperty property;

            if (!component->getProperty(index, property)) {
                continue;
            }

            if (property.value == nullptr || property.readOnly || !property.serializable || property.type != propertySnapshot.type || std::strcmp(property.key, propertySnapshot.key.c_str()) != 0) {
                continue;
            }

            ApplyPropertyValue(property, propertySnapshot.value);

            component->notifyPropertyChanged(propertySnapshot.key.c_str());

            break;
        }
    }
}

void CaptureObject(const Object &object, std::vector<ObjectSnapshot> &snapshots)
{
    ObjectSnapshot snapshot;

    snapshot.id = object.getId();
    snapshot.name = object.getName();
    snapshot.active = object.isActive();
    snapshot.layer = object.getLayer();
    snapshot.transform = object.transform;

    for (std::size_t index = 0; index < object.getComponentCount(); ++index) {
        Component *component = object.getComponentAt(index);

        if (component != nullptr) {
            snapshot.components.push_back(CaptureComponent(*component));
        }
    }

    snapshots.push_back(std::move(snapshot));

    for (std::size_t index = 0; index < object.getChildCount(); ++index) {
        const Object *child = object.getChild(index);

        if (child != nullptr) {
            CaptureObject(*child, snapshots);
        }
    }
}
} // namespace

struct EditorPlaySnapshot::Impl
{
    std::vector<ObjectSnapshot> objects;
    bool captured = false;
};

EditorPlaySnapshot::EditorPlaySnapshot() : impl(std::make_unique<Impl>()) {}

EditorPlaySnapshot::~EditorPlaySnapshot() = default;

void EditorPlaySnapshot::capture(const Scene &scene)
{
    impl->objects.clear();

    for (std::size_t index = 0; index < scene.getRootObjectCount(); ++index) {
        const Object *object = scene.getRootObject(index);

        if (object != nullptr) {
            CaptureObject(*object, impl->objects);
        }
    }

    impl->captured = true;
}

void EditorPlaySnapshot::restore(Scene &scene)
{
    if (!impl->captured) {
        return;
    }

    for (const ObjectSnapshot &snapshot : impl->objects) {
        Object *object = scene.findObject(snapshot.id);

        if (object == nullptr) {
            continue;
        }

        object->setName(snapshot.name);
        object->setActive(snapshot.active);
        object->setLayer(snapshot.layer);

        // First restore the transform. Rigidbody notification
        // clears runtime momentum and teleports the PhysX actor.
        object->transform = snapshot.transform;

        object->notifyTransformChanged();

        // Then restore component configuration and initial
        // velocities so they are not cleared by the teleport.
        for (const ComponentSnapshot &componentSnapshot : snapshot.components) {
            RestoreComponent(*object, componentSnapshot);
        }
    }

    clear();
}

void EditorPlaySnapshot::clear()
{
    impl->objects.clear();
    impl->captured = false;
}

bool EditorPlaySnapshot::isCaptured() const { return impl->captured; }