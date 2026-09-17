#pragma once

#include <glm.hpp>

#include "engineApi.hpp"

class RenderTarget;

struct GameViewPanelState
{
    int requestedWidth = 960;
    int requestedHeight = 540;

    bool hovered = false;
    bool focused = false;
    bool fullscreen = false;

    float imageMinimumX = 0.0f;
    float imageMinimumY = 0.0f;
    float imageMaximumX = 0.0f;
    float imageMaximumY = 0.0f;
};

ENGINE_API void DrawGameViewPanel(const RenderTarget &renderTarget, GameViewPanelState &state);

ENGINE_API bool IsPointInsideGameView(const GameViewPanelState &state, float windowX, float windowY);

ENGINE_API glm::vec2 MapPointToGameView(const GameViewPanelState &state, float windowX, float windowY, int renderTargetWidth, int renderTargetHeight);
