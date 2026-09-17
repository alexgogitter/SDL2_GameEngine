#include "object.hpp"

#include <algorithm>
#include <cstdio>

#include <gtc/quaternion.hpp>

#include "colliderComponent.hpp"
#include "meshRendererComponent.hpp"
#include "spriteRendererComponent.hpp"

namespace
{
constexpr int DebugColliderRenderLayer = 9000;
}

Object::Object(const std::string &objectName, Resource_manager &resourceManager, Renderer2D *objectRenderer, Renderer3D *objectRenderer3D, ObjectId requestedId) : id(AcquireObjectId(requestedId)), name(objectName), renderer(objectRenderer), renderer3D(objectRenderer3D) { (void)resourceManager; }

ObjectId Object::getId() const { return id; }

const std::string &Object::getName() const { return name; }

void Object::setName(const std::string &newName) { name = newName; }

bool Object::isActive() const { return active; }

void Object::setActive(bool value) { active = value; }

int Object::getLayer() const { return layer; }

bool Object::setLayer(int newLayer)
{
    if (newLayer < 0 || newLayer >= 32) {
        return false;
    }

    layer = newLayer;
    return true;
}

Object::~Object()
{
    for (const auto &pair : components) {
        pair.second->destroy();
        delete pair.second;
    }

    for (Object *child : child_Objects) {
        child->parentObject = nullptr;
        delete child;
    }
}

void Object::update(std::uint64_t deltaTime)
{
    if (!active) {
        return;
    }

    Update(deltaTime);

    for (const auto &pair : components) {
        pair.second->preUpdate(deltaTime);
    }

    for (const auto &pair : components) {
        pair.second->update(deltaTime);
    }

    for (const auto &pair : components) {
        pair.second->postUpdate(deltaTime);
    }

    for (Object *child : child_Objects) {
        child->update(deltaTime);
    }
}

void Object::draw2D(Renderer2D *r)
{

    if (!active) {
        return;
    }

    if (r == nullptr) {
        std::fprintf(stderr, "ERROR: Renderer is null in Object::draw().\n");
        return;
    }

    const glm::vec4 normalisedColour = draw_colour / 255.0f;
    const bool hasRendererComponent = getComponent<SpriteRendererComponent>() != nullptr || getComponent<MeshRendererComponent>() != nullptr;

    // if (
    //     !hasRendererComponent &&
    //     normalisedColour.a > 0.0f
    // )
    // {
    //     const Transform2D worldDrawTransform = getWorldTransform().toTransform2D();
    //     r->SubmitSolidQuad2D(
    //         worldDrawTransform,
    //         normalisedColour,
    //         0,
    //         layer
    //     );
    // }

    // Rendering is component-driven. A SpriteRendererComponent draws from its
    // own local draw rect while sharing this object's position and rotation.
    for (const auto &pair : components) {
        pair.second->draw2D(r);
    }

#ifndef NDEBUG
    if (ColliderComponent *collider = getComponent<ColliderComponent>()) {
        Transform2D colliderTransform = getWorldTransform().toTransform2D();
        colliderTransform.setScale(collider->getSizePixels() * 0.5f);
        r->SubmitOutline2D(colliderTransform, glm::vec4(0.08f, 0.85f, 1.0f, 1.0f), DebugColliderRenderLayer, layer);
    }
#endif

    for (Object *child : child_Objects) {
        child->draw2D(r);
    }
}

void Object::draw3D(Renderer3D *r)
{
    if (!active || r == nullptr) {
        return;
    }

    for (const auto &pair : components) {
        pair.second->draw3D(r);
    }

    for (Object *child : child_Objects) {
        child->draw3D(r);
    }
}

Component *Object::add_Component(Component *component)
{
    if (component == nullptr) {
        return nullptr;
    }
    const auto existing = components.find(component->getTypeName());
    if (existing != components.end()) {
        existing->second->destroy();
        delete existing->second;

        existing->second = component;
        component->create();
        return component;
    }

    components[component->getTypeName()] = component;
    component->create();
    return component;
}

std::size_t Object::getComponentCount() const { return components.size(); }

Component *Object::getComponentAt(std::size_t index) const
{
    if (index >= components.size()) {
        return nullptr;
    }

    auto iterator = components.begin();

    for (std::size_t current = 0; current < index; ++current) {
        ++iterator;
    }

    return iterator->second;
}

Component *Object::getComponentByTypeName(const std::string &typeName) const
{
    const auto found = components.find(typeName);
    printf("Searching for component type: %s\n", typeName.c_str());
    printf("Found component type: %s\n", found != components.end() ? found->second->getTypeName().c_str() : "null");
    return found != components.end() ? found->second : nullptr;
}

int Object::remove_Component(Component *component)
{
    if (component == nullptr) {
        return -1;
    }

    return remove_Component(component->getTypeName());
}

int Object::remove_Component(const std::string &componentName)
{
    const auto found = components.find(componentName);
    if (found == components.end()) {
        return -1;
    }
    found->second->destroy();
    delete found->second;
    components.erase(found);
    return 0;
}

int Object::add_Child_Object(Object *object)
{
    if (object == nullptr || object == this || object->parentObject != nullptr) {
        return -1;
    }

    // Reject attaching one of our ancestors beneath us.
    if (object->findInHierarchy(id) != nullptr) {
        return -1;
    }

    object->parentObject = this;
    child_Objects.push_back(object);
    return 0;
}

void Object::notifyTransformChanged()
{
    for (const auto &entry : components) {
        if (entry.second != nullptr) {
            entry.second->notifyTransformChanged();
        }
    }
}

int Object::remove_Child_Object(Object *object)
{
    Object *detached = detachChild(object);

    if (detached == nullptr) {
        return -1;
    }

    delete detached;
    return 0;
}

Object *Object::detachChild(Object *object)
{
    const auto found = std::find(child_Objects.begin(), child_Objects.end(), object);

    if (found == child_Objects.end()) {
        return nullptr;
    }

    Object *detached = *found;

    child_Objects.erase(found);
    detached->parentObject = nullptr;

    return detached;
}

Object *Object::getParentObject() const { return parentObject; }

std::size_t Object::getChildCount() const { return child_Objects.size(); }

Object *Object::getChild(std::size_t index) const
{
    if (index >= child_Objects.size()) {
        return nullptr;
    }

    return child_Objects[index];
}

Object *Object::findInHierarchy(ObjectId objectId)
{
    if (id == objectId) {
        return this;
    }

    for (Object *child : child_Objects) {
        if (Object *found = child->findInHierarchy(objectId)) {
            return found;
        }
    }

    return nullptr;
}

glm::mat4 Object::getWorldMatrix() const
{
    const glm::mat4 localMatrix = transform.getModelMatrix();

    return parentObject != nullptr ? parentObject->getWorldMatrix() * localMatrix : localMatrix;
}

Transform3D Object::getWorldTransform() const
{
    Transform3D result;
    result.setFromMatrix(getWorldMatrix());
    return result;
}

glm::vec3 Object::getWorldPosition() const { return glm::vec3(getWorldMatrix()[3]); }

glm::quat Object::getWorldRotation() const
{
    if (parentObject == nullptr) {
        return transform.getRotation();
    }

    return glm::normalize(parentObject->getWorldRotation() * transform.getRotation());
}
