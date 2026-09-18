#include "editorDockspace.hpp"

#include <imgui.h>
#include <initializer_list>
#include <imgui_internal.h>

void DrawEditorDockspace()
{
    ImGuiViewport *viewport = ImGui::GetMainViewport();

    const ImGuiID dockspaceId = ImHashStr("MainEditorDockspace");

    const ImGuiDockNodeFlags flags = ImGuiDockNodeFlags_PassthruCentralNode;

    // Only create the default layout when no saved
    // docking layout exists in imgui.ini.
    if (ImGui::DockBuilderGetNode(dockspaceId) == nullptr) {
        ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace | flags);

        ImGui::DockBuilderSetNodePos(dockspaceId, viewport->WorkPos);

        ImGui::DockBuilderSetNodeSize(dockspaceId, viewport->WorkSize);

        ImGuiID centreDock = dockspaceId;
        ImGuiID leftDock = 0;
        ImGuiID rightDock = 0;
        ImGuiID bottomDock = 0;

        ImGui::DockBuilderSplitNode(centreDock, ImGuiDir_Left, 0.18f, &leftDock, &centreDock);

        ImGui::DockBuilderSplitNode(centreDock, ImGuiDir_Right, 0.24f, &rightDock, &centreDock);

        ImGui::DockBuilderSplitNode(centreDock, ImGuiDir_Down, 0.24f, &bottomDock, &centreDock);

        ImGui::DockBuilderDockWindow("Hierarchy", leftDock);

        ImGui::DockBuilderDockWindow("Inspector", rightDock);

        ImGui::DockBuilderDockWindow("Scene View", centreDock);

        ImGui::DockBuilderDockWindow("Game View", centreDock);

        ImGui::DockBuilderDockWindow("Camera Outputs", bottomDock);
        ImGui::DockBuilderDockWindow("Assets", bottomDock);
        ImGui::DockBuilderDockWindow("Log", bottomDock);

        ImGui::DockBuilderFinish(dockspaceId);
    }

    // Introduce new panels into an existing layout without resetting the user's arrangement.
    if (auto *settings = ImGui::FindWindowSettingsByID(ImHashStr("Camera Outputs"))) {
        if (settings->DockId != 0) {
            for (const char *panel : {"Assets", "Log"}) {
                if (!ImGui::FindWindowSettingsByID(ImHashStr(panel)))
                    ImGui::DockBuilderDockWindow(panel, settings->DockId);
            }
        }
    }
    ImGui::DockSpaceOverViewport(dockspaceId, viewport, flags);
}