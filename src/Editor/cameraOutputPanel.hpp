#pragma once

#include "engineApi.hpp"

class CameraSystem;
class Scene;

/// Draws camera selection controls for editor outputs.
ENGINE_API void DrawCameraOutputPanel(CameraSystem &cameras, Scene &scene);