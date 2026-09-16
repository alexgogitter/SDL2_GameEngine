#pragma once

#include "engineApi.hpp"

class Camera;
class EditorSelection;
class Scene;

ENGINE_API bool DrawEditorGizmoOverlay(
    Scene& scene,
    EditorSelection& selection,
    const Camera& camera,
    float imageMinimumX,
    float imageMinimumY,
    float imageMaximumX,
    float imageMaximumY,
    bool viewportHovered
);