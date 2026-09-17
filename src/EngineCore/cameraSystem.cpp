#include "cameraSystem.hpp"

#include <algorithm>
#include <vector>

#include "cameraComponent.hpp"
#include "object.hpp"
#include "scene.hpp"

namespace
{
std::size_t OutputIndex(CameraOutputTarget output) { return static_cast<std::size_t>(output); }

bool IsOutputValid(CameraOutputTarget output)
{
    const int value = static_cast<int>(output);

    return value >= static_cast<int>(CameraOutputTarget::GameView) && value <= static_cast<int>(CameraOutputTarget::Disabled);
}

bool IsActiveInHierarchy(const Object *object)
{
    for (const Object *current = object; current != nullptr; current = current->getParentObject()) {
        if (!current->isActive()) {
            return false;
        }
    }

    return true;
}

void CollectFromObject(Object *object, CameraOutputTarget output, std::vector<CameraComponent *> &cameras)
{
    if (object == nullptr) {
        return;
    }

    auto *camera = object->getComponent<CameraComponent>();

    if (camera != nullptr && camera->isEnabled() && IsActiveInHierarchy(object) && camera->getOutputTarget() == output) {
        cameras.push_back(camera);
    }

    for (std::size_t index = 0; index < object->getChildCount(); ++index) {
        CollectFromObject(object->getChild(index), output, cameras);
    }
}

std::vector<CameraComponent *> CollectCameras(Scene &scene, CameraOutputTarget output)
{
    std::vector<CameraComponent *> cameras;

    if (!IsOutputValid(output) || output == CameraOutputTarget::Disabled) {
        return cameras;
    }

    for (std::size_t index = 0; index < scene.getRootObjectCount(); ++index) {
        CollectFromObject(scene.getRootObject(index), output, cameras);
    }

    std::stable_sort(cameras.begin(), cameras.end(), [](const CameraComponent *left, const CameraComponent *right) {
        if (left->getPriority() != right->getPriority()) {
            return left->getPriority() > right->getPriority();
        }

        const Object *leftObject = left->getParent();

        const Object *rightObject = right->getParent();

        if (leftObject == nullptr || rightObject == nullptr) {
            return leftObject != nullptr;
        }

        return leftObject->getId() < rightObject->getId();
    });

    return cameras;
}

CameraComponent *FindEligibleCamera(Scene &scene, CameraOutputTarget output, ObjectId objectId)
{
    Object *object = scene.findObject(objectId);

    if (object == nullptr || !IsActiveInHierarchy(object)) {
        return nullptr;
    }

    auto *camera = object->getComponent<CameraComponent>();

    if (camera == nullptr || !camera->isEnabled() || camera->getOutputTarget() != output) {
        return nullptr;
    }

    return camera;
}
} // namespace

CameraSystem::CameraSystem() { primaryCameraIds.fill(InvalidObjectId); }

bool CameraSystem::setPrimaryCamera(Scene &scene, CameraOutputTarget output, ObjectId cameraObjectId)
{
    if (!IsOutputValid(output) || output == CameraOutputTarget::Disabled) {
        return false;
    }

    CameraComponent *camera = FindEligibleCamera(scene, output, cameraObjectId);

    if (camera == nullptr) {
        return false;
    }

    primaryCameraIds[OutputIndex(output)] = cameraObjectId;

    return true;
}

void CameraSystem::clearPrimaryCamera(CameraOutputTarget output)
{
    if (!IsOutputValid(output)) {
        return;
    }

    primaryCameraIds[OutputIndex(output)] = InvalidObjectId;
}

ObjectId CameraSystem::getPrimaryCameraId(CameraOutputTarget output) const
{
    if (!IsOutputValid(output)) {
        return InvalidObjectId;
    }

    return primaryCameraIds[OutputIndex(output)];
}

CameraComponent *CameraSystem::resolvePrimaryCamera(Scene &scene, CameraOutputTarget output)
{
    if (!IsOutputValid(output) || output == CameraOutputTarget::Disabled) {
        return nullptr;
    }

    const std::size_t outputIndex = OutputIndex(output);

    CameraComponent *selected = FindEligibleCamera(scene, output, primaryCameraIds[outputIndex]);

    if (selected != nullptr) {
        return selected;
    }

    primaryCameraIds[outputIndex] = InvalidObjectId;

    std::vector<CameraComponent *> cameras = CollectCameras(scene, output);

    if (cameras.empty()) {
        return nullptr;
    }

    CameraComponent *fallback = cameras.front();

    Object *owner = fallback->getParent();

    if (owner != nullptr) {
        primaryCameraIds[outputIndex] = owner->getId();
    }

    return fallback;
}

std::size_t CameraSystem::getCameraCount(Scene &scene, CameraOutputTarget output) const { return CollectCameras(scene, output).size(); }

CameraComponent *CameraSystem::getCameraAt(Scene &scene, CameraOutputTarget output, std::size_t index) const
{
    std::vector<CameraComponent *> cameras = CollectCameras(scene, output);

    return index < cameras.size() ? cameras[index] : nullptr;
}