#include "scene.hpp"

#include "object.hpp"
#include "render2D.hpp"
#include "render3D.hpp"
#include "resource_manager.hpp"

#include <memory>
#include <vector>
#include <stdexcept>

struct Scene::Impl
{
    Resource_manager *resources = nullptr;
    Renderer2D *renderer = nullptr;
    Renderer3D *renderer3D = nullptr;
    std::vector<std::unique_ptr<Object>> rootObjects;
};

namespace
{
bool destroyDescendant(Object *parent, ObjectId objectId)
{
    if (parent == nullptr) {
        return false;
    }

    for (std::size_t index = 0; index < parent->getChildCount(); ++index) {
        Object *child = parent->getChild(index);

        if (child->getId() == objectId) {
            return parent->remove_Child_Object(child) == 0;
        }

        if (destroyDescendant(child, objectId)) {
            return true;
        }
    }

    return false;
}
} // namespace

Scene::Scene(Resource_manager &resources, Renderer2D &renderer, Renderer3D &renderer3D) : impl(std::make_unique<Impl>())
{
    impl->resources = &resources;
    impl->renderer = &renderer;
    impl->renderer3D = &renderer3D;
}

Scene::~Scene() = default;

void Scene::clear() { impl->rootObjects.clear(); }

void Scene::swapContents(Scene &other)
{
    if (impl->resources != other.impl->resources || impl->renderer != other.impl->renderer || impl->renderer3D != other.impl->renderer3D)
        throw std::invalid_argument("Scene services must match");
    impl->rootObjects.swap(other.impl->rootObjects);
}

void Scene::appendContents(Scene &other)
{
    if (this == &other) return;
    if (impl->resources != other.impl->resources || impl->renderer != other.impl->renderer || impl->renderer3D != other.impl->renderer3D)
        throw std::invalid_argument("Scene services must match");
    impl->rootObjects.reserve(impl->rootObjects.size() + other.impl->rootObjects.size());
    for (auto &root : other.impl->rootObjects) impl->rootObjects.push_back(std::move(root));
    other.impl->rootObjects.clear();
}

Object *Scene::createObject(const std::string &name, ObjectId requestedId)
{
    if (requestedId != InvalidObjectId && findObject(requestedId) != nullptr) {
        return nullptr;
    }

    auto object = std::make_unique<Object>(name, *impl->resources, impl->renderer, impl->renderer3D, requestedId);

    Object *result = object.get();
    impl->rootObjects.push_back(std::move(object));
    return result;
}

Object *Scene::createChildObject(ObjectId parentId, const std::string &name, ObjectId requestedId)
{
    Object *parent = findObject(parentId);

    if (parent == nullptr || (requestedId != InvalidObjectId && findObject(requestedId) != nullptr)) {
        return nullptr;
    }

    auto child = std::make_unique<Object>(name, *impl->resources, impl->renderer, impl->renderer3D, requestedId);

    Object *result = child.get();

    if (parent->add_Child_Object(child.release()) != 0) {
        return nullptr;
    }

    return result;
}

Object *Scene::findObject(ObjectId objectId)
{
    if (objectId == InvalidObjectId) {
        return nullptr;
    }

    for (const auto &root : impl->rootObjects) {
        if (Object *found = root->findInHierarchy(objectId)) {
            return found;
        }
    }

    return nullptr;
}

bool Scene::destroyObject(ObjectId objectId)
{
    if (objectId == InvalidObjectId) {
        return false;
    }

    for (auto iterator = impl->rootObjects.begin(); iterator != impl->rootObjects.end(); ++iterator) {
        if ((*iterator)->getId() == objectId) {
            impl->rootObjects.erase(iterator);
            return true;
        }
    }

    for (const auto &root : impl->rootObjects) {
        if (destroyDescendant(root.get(), objectId)) {
            return true;
        }
    }

    return false;
}

std::size_t Scene::getRootObjectCount() const { return impl->rootObjects.size(); }

Object *Scene::getRootObject(std::size_t index) const
{
    if (index >= impl->rootObjects.size()) {
        return nullptr;
    }

    return impl->rootObjects[index].get();
}

void Scene::update(std::uint64_t deltaTime)
{
    for (const auto &root : impl->rootObjects) {
        root->update(deltaTime);
    }
}

void Scene::draw2D()
{
    for (const auto &root : impl->rootObjects) {
        root->draw2D(impl->renderer);
    }
}

void Scene::draw3D()
{
    for (const auto &root : impl->rootObjects) {
        root->draw3D(impl->renderer3D);
    }
}

bool Scene::reparentObject(ObjectId objectId, ObjectId newParentId, bool keepWorldTransform)
{
    Object *object = findObject(objectId);

    Object *newParent = newParentId != InvalidObjectId ? findObject(newParentId) : nullptr;

    if (object == nullptr) {
        return false;
    }

    if (newParentId != InvalidObjectId && newParent == nullptr) {
        return false;
    }

    if (object == newParent) {
        return false;
    }

    // A descendant cannot become its ancestor's parent.
    if (newParent != nullptr && object->findInHierarchy(newParentId) != nullptr) {
        return false;
    }

    Object *previousParent = object->getParentObject();

    if (previousParent == newParent) {
        return true;
    }

    Transform3D newLocalTransform = object->transform;

    if (keepWorldTransform) {
        glm::mat4 localMatrix = object->getWorldMatrix();

        if (newParent != nullptr) {
            const glm::mat4 parentWorld = newParent->getWorldMatrix();

            if (std::abs(glm::determinant(parentWorld)) <= 0.000001f) {
                return false;
            }

            localMatrix = glm::inverse(parentWorld) * localMatrix;
        }

        if (!newLocalTransform.setFromMatrix(localMatrix)) {
            return false;
        }
    }

    std::unique_ptr<Object> ownership;

    if (previousParent != nullptr) {
        ownership.reset(previousParent->detachChild(object));
    }
    else {
        for (auto iterator = impl->rootObjects.begin(); iterator != impl->rootObjects.end(); ++iterator) {
            if (iterator->get() == object) {
                ownership = std::move(*iterator);
                impl->rootObjects.erase(iterator);
                break;
            }
        }
    }

    if (ownership == nullptr) {
        return false;
    }

    if (newParent != nullptr) {
        if (newParent->add_Child_Object(ownership.get()) != 0) {
            // Restore ownership if an unexpected
            // attachment failure occurs.
            if (previousParent != nullptr) {
                if (previousParent->add_Child_Object(ownership.get()) == 0) {
                    ownership.release();
                }
            }
            else {
                impl->rootObjects.push_back(std::move(ownership));
            }

            return false;
        }

        ownership.release();
    }
    else {
        impl->rootObjects.push_back(std::move(ownership));
    }

    object->transform = newLocalTransform;
    return true;
}
