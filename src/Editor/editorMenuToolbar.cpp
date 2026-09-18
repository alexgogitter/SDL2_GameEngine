#include "editorMenuToolbar.hpp"

#include <imgui.h>
#include "editorPreferences.hpp"
#include "editorProject.hpp"

#include <cstdio>

void LoadPreferencesIntoUI() 
{
    auto& prefs = EditorPreferences::getInstance();
    // Load with default fallbacks if they don't exist yet
    g_preferencesUIState.showOnStartup = prefs.getPreference<bool>("ShowConsole");
    g_preferencesUIState.font_scale = prefs.getPreference<float>("FontScale");
    g_preferencesUIState.theme_index = prefs.getPreference<int>("ThemeIndex");
    g_preferencesUIState.autoSaveEnabled = prefs.getPreference<bool>("AutoSaveEnabled");
    g_preferencesUIState.autoSaveInterval = prefs.getPreference<int>("AutoSaveInterval");
    std::snprintf(g_preferencesUIState.externalEditor, sizeof(g_preferencesUIState.externalEditor), "%s",
                  prefs.getPreference<std::string>("ExternalEditor").c_str());
 
}

void SavePreferencesFromUI()
{
    auto& prefs = EditorPreferences::getInstance();
    prefs.setPreference("ShowConsole", g_preferencesUIState.showOnStartup);
    prefs.setPreference("FontScale", g_preferencesUIState.font_scale);
    prefs.setPreference("ThemeIndex", g_preferencesUIState.theme_index);
    prefs.setPreference("AutoSaveEnabled", g_preferencesUIState.autoSaveEnabled);   
    prefs.setPreference("AutoSaveInterval", g_preferencesUIState.autoSaveInterval);
    prefs.setPreference("ExternalEditor", std::string(g_preferencesUIState.externalEditor));
    // Save to disk
    prefs.saveToFile("editor_prefs.json"); 
}

void RenderPreferencesWindow(PreferencesUIState &ui_state, char *search_buffer) {
    if (!ui_state.showPreferencesModal) return;

    // Set a default size for the window, but allow resizing
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    
    // Center the window on first use
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));

    ImGui::SetNextWindowDockID(0);
    ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
    if (ImGui::Begin("Preferences", &ui_state.showPreferencesModal, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking)) {
        
        // --- TOP: Search Bar (Optional, makes it look like IntelliJ) ---
        ImGui::InputTextWithHint("##Search", "Search settings...", search_buffer, IM_ARRAYSIZE(search_buffer));
        ImGui::Separator();

        // --- LEFT SIDEBAR: Categories ---
        // Width: 200px, Height: Leave room for bottom buttons (-30px)
        ImGui::BeginChild("Categories", ImVec2(200, -30.0f), true);
        {
            if (ImGui::Selectable("General", ui_state.selectedTabIndex == 0)) ui_state.selectedTabIndex = 0;
            if (ImGui::Selectable("Appearance", ui_state.selectedTabIndex == 1)) ui_state.selectedTabIndex = 1;
            if (ImGui::Selectable("Editor", ui_state.selectedTabIndex == 2)) ui_state.selectedTabIndex = 2;
            if (ImGui::Selectable("Keymap", ui_state.selectedTabIndex == 3)) ui_state.selectedTabIndex = 3;
        }

        ImGui::EndChild();
        ImGui::SameLine();

        // --- RIGHT PANEL: Content ---
        ImGui::BeginChild("Content", ImVec2(0, -30.0f), true); // 0 width means take remaining space
        {
            // Add some padding inside the content area
            ImGui::Indent(10.0f);
            ImGui::Spacing();

            if (ui_state.selectedTabIndex == 0)
            {
                // General Settings
                ImGui::Text("General Settings");
                ImGui::SeparatorText("Startup");

                
            } 
            else if (ui_state.selectedTabIndex == 1)
            {
               // Appearance Settings
            }
            else if (ui_state.selectedTabIndex == 2)
            {
                ImGui::TextUnformatted("Script Editor");
                ImGui::InputText("External editor executable", ui_state.externalEditor, IM_ARRAYSIZE(ui_state.externalEditor));
                ImGui::TextDisabled("Leave blank to use VS Code's 'code' command. Set a full .exe path for another editor.");
            }
            else if (ui_state.selectedTabIndex == 3)
            {
                // Keymap Settings                
            }

            ImGui::Unindent(10.0f);
        }
        ImGui::EndChild();

        // --- BOTTOM: Action Buttons ---
        ImGui::Separator();
        
        // Push buttons to the right side
        float button_width = 80.0f;
        float window_width = ImGui::GetWindowSize().x;
        ImGui::SetCursorPosX(window_width - (button_width * 3) - 20.0f); // 20px padding

        if (ImGui::Button("OK", ImVec2(button_width, 0))) {
            SavePreferencesFromUI();
            ui_state.showPreferencesModal = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(button_width, 0))) {
            // Revert changes by reloading from preferences
            LoadPreferencesIntoUI();
            ui_state.showPreferencesModal = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Apply", ImVec2(button_width, 0))) {
            SavePreferencesFromUI();
            // Keep window open
        }
    }
    ImGui::End();
}

void DrawMenuToolbar(EditorProject &project)
{
    if(ImGui::BeginMainMenuBar())
    {
        if(ImGui::BeginMenu("File"))
        {
            project.drawFileMenu();
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Edit"))
        {
            if(ImGui::MenuItem("Undo", "Ctrl+Z"))
            {
                // Handle undo action
            }
            if(ImGui::MenuItem("Redo", "Ctrl+Shift+Z"))
            {
                // Handle redo action
            }
            if(ImGui::MenuItem("Preferences", "Ctrl+,"))
            {
                g_preferencesUIState.showPreferencesModal = true;
                LoadPreferencesIntoUI();
            }
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("View"))
        {
            if(ImGui::MenuItem("Toggle Fullscreen", "F11"))
            {
                // Handle toggle fullscreen action
            }
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Help"))
        {
            if(ImGui::MenuItem("About"))
            {
                // Handle about action
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();

        RenderPreferencesWindow(g_preferencesUIState, searchBuffer);
    }
}

