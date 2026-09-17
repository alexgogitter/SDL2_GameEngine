#include "gameViewPanel.hpp"

#include <algorithm>
#include <cstdint>

#include <imgui.h>

#include "renderTarget.hpp"

void DrawGameViewPanel(const RenderTarget &renderTarget, GameViewPanelState &state)
{
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

    bool zeroWindowPadding = false;

    const char *windowName = "Game View";

    if (state.fullscreen) {
        ImGuiViewport *viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(viewport->Pos);

        ImGui::SetNextWindowSize(viewport->Size);

        flags |= ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

        windowName = "Game View Fullscreen";

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        zeroWindowPadding = true;
    }

    const bool visible = ImGui::Begin(windowName, nullptr, flags);

    if (zeroWindowPadding) {
        ImGui::PopStyleVar();
    }

    if (!visible) {
        state.hovered = false;
        state.focused = false;
        ImGui::End();
        return;
    }

    const ImVec2 available = ImGui::GetContentRegionAvail();

    if (available.x >= 16.0f && available.y >= 16.0f) {
        state.requestedWidth = static_cast<int>(available.x);

        state.requestedHeight = static_cast<int>(available.y);
    }

    state.hovered = false;

    if (renderTarget.isValid() && available.x > 0.0f && available.y > 0.0f) {
        const ImVec2 imageMinimum = ImGui::GetCursorScreenPos();

        const ImVec2 imageMaximum(imageMinimum.x + available.x, imageMinimum.y + available.y);

        const ImTextureID texture = static_cast<ImTextureID>(static_cast<intptr_t>(renderTarget.getColourTextureId()));

        ImGui::Image(texture, available, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

        state.imageMinimumX = imageMinimum.x;
        state.imageMinimumY = imageMinimum.y;
        state.imageMaximumX = imageMaximum.x;
        state.imageMaximumY = imageMaximum.y;

        state.hovered = ImGui::IsItemHovered();

        state.focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

        ImDrawList *drawList = ImGui::GetWindowDrawList();

        drawList->AddRect(imageMinimum, imageMaximum, state.hovered ? IM_COL32(90, 170, 255, 255) : IM_COL32(65, 70, 80, 255));

        drawList->AddText(ImVec2(imageMinimum.x + 10.0f, imageMinimum.y + 10.0f), IM_COL32(235, 240, 250, 255), state.fullscreen ? "F11 or Escape: Exit Fullscreen" : "F11: Fullscreen Game View");
    }
    else {
        state.focused = false;

        ImGui::TextDisabled("Game render target unavailable");
    }

    ImGui::End();
}

bool IsPointInsideGameView(const GameViewPanelState &state, float windowX, float windowY) { return state.hovered && windowX >= state.imageMinimumX && windowX <= state.imageMaximumX && windowY >= state.imageMinimumY && windowY <= state.imageMaximumY; }

glm::vec2 MapPointToGameView(const GameViewPanelState &state, float windowX, float windowY, int renderTargetWidth, int renderTargetHeight)
{
    const float displayedWidth = std::max(state.imageMaximumX - state.imageMinimumX, 1.0f);

    const float displayedHeight = std::max(state.imageMaximumY - state.imageMinimumY, 1.0f);

    const float normalisedX = std::clamp((windowX - state.imageMinimumX) / displayedWidth, 0.0f, 1.0f);

    const float normalisedY = std::clamp((windowY - state.imageMinimumY) / displayedHeight, 0.0f, 1.0f);

    return {normalisedX * static_cast<float>(std::max(renderTargetWidth, 1)),

            normalisedY * static_cast<float>(std::max(renderTargetHeight, 1))};
}