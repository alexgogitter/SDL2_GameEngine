#pragma once

#include "engineApi.hpp"

class EditorCamera;
class EditorSelection;
class RenderTarget;
class Scene;

struct SceneViewPanelState
{
    int requestedWidth = 960;
    int requestedHeight = 540;

    bool hovered = false;
    bool focused = false;

    float imageMinimumX = 0.0f;
    float imageMinimumY = 0.0f;
    float imageMaximumX = 0.0f;
    float imageMaximumY = 0.0f;
};

ENGINE_API void DrawSceneViewPanel(const RenderTarget &renderTarget, EditorCamera &editorCamera, Scene &scene, EditorSelection &selection, SceneViewPanelState &state);