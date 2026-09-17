#pragma once

#include <array>
#include <cstddef>

#include "engineApi.hpp"
#include "objectId.hpp"

class CameraComponent;
class Scene;

enum class CameraOutputTarget : int;

class CameraSystem
{
  public:
    ENGINE_API CameraSystem();

    /// Explicitly selects a camera for an output.
    ENGINE_API bool setPrimaryCamera(Scene &scene, CameraOutputTarget output, ObjectId cameraObjectId);

    ENGINE_API void clearPrimaryCamera(CameraOutputTarget output);

    ENGINE_API ObjectId getPrimaryCameraId(CameraOutputTarget output) const;

    /// Resolves the explicit camera or falls back to the
    /// highest-priority eligible camera.
    ENGINE_API CameraComponent *resolvePrimaryCamera(Scene &scene, CameraOutputTarget output);

    ENGINE_API std::size_t getCameraCount(Scene &scene, CameraOutputTarget output) const;

    ENGINE_API CameraComponent *getCameraAt(Scene &scene, CameraOutputTarget output, std::size_t index) const;

  private:
    static constexpr std::size_t OutputCount = 4;

    std::array<ObjectId, OutputCount> primaryCameraIds;
};