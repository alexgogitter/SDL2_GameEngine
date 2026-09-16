#include "editorPlayToolbar.hpp"

#include <imgui.h>

#include "editorPlayState.hpp"

void DrawEditorPlayToolbar(
    EditorPlayState& playState
)
{
    if (!ImGui::BeginMainMenuBar())
    {
        return;
    }

    ImGui::TextUnformatted("Simulation");
    ImGui::Separator();

    if (playState.isEditing())
    {
        if (ImGui::Button("Play"))
        {
            playState.enterPlayMode();
        }

        ImGui::SameLine();

        ImGui::TextColored(
            ImVec4(0.55f, 0.75f, 1.0f, 1.0f),
            "Edit Mode"
        );
    }
    else
    {
        if (ImGui::Button("Stop"))
        {
            playState.stop();
        }

        ImGui::SameLine();

        if (ImGui::Button(
            playState.isPaused()
                ? "Resume"
                : "Pause"
        ))
        {
            playState.togglePause();
        }

        ImGui::SameLine();

        ImGui::BeginDisabled(
            !playState.isPaused()
        );

        if (ImGui::Button("Step"))
        {
            playState.requestSingleStep();
        }

        ImGui::EndDisabled();

        ImGui::SameLine();

        if (playState.isPaused())
        {
            ImGui::TextColored(
                ImVec4(1.0f, 0.72f, 0.25f, 1.0f),
                "Paused"
            );
        }
        else
        {
            ImGui::TextColored(
                ImVec4(0.35f, 1.0f, 0.45f, 1.0f),
                "Playing"
            );
        }
    }

    ImGui::EndMainMenuBar();
}