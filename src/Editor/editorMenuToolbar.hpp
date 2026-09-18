#pragma once
#include "engineApi.hpp"

class EditorProject;
ENGINE_API void DrawMenuToolbar(EditorProject &project);

struct PreferencesUIState
{
    bool showPreferencesModal = false;
    int selectedTabIndex = 0;

    bool showOnStartup = false;
    bool autoSaveEnabled = false;
    int autoSaveInterval = 5; // in minutes

    float font_scale = 1.0f;
    unsigned int theme_index = 0;
    char externalEditor[1024] = "";
    
};


static PreferencesUIState g_preferencesUIState;
static const char *g_preferencesTabNames[] = {
    "General",
    "Editor",
    "Shortcuts",
    "About"
};

static char searchBuffer[256] = "";

void loadPreferencesUI();
void savePreferencesUI();
