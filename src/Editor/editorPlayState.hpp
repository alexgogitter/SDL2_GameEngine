#pragma once

enum class EditorPlayMode
{
    Edit,
    Playing,
    Paused
};

class EditorPlayState final
{
  public:
    EditorPlayMode getMode() const { return mode; }

    bool isEditing() const { return mode == EditorPlayMode::Edit; }

    bool isPlaying() const { return mode == EditorPlayMode::Playing; }

    bool isPaused() const { return mode == EditorPlayMode::Paused; }

    void enterPlayMode()
    {
        mode = EditorPlayMode::Playing;
        singleStepRequested = false;
    }

    void stop()
    {
        mode = EditorPlayMode::Edit;
        singleStepRequested = false;
    }

    void togglePause()
    {
        if (mode == EditorPlayMode::Playing) {
            mode = EditorPlayMode::Paused;
        }
        else if (mode == EditorPlayMode::Paused) {
            mode = EditorPlayMode::Playing;
        }

        singleStepRequested = false;
    }

    void requestSingleStep()
    {
        if (mode == EditorPlayMode::Paused) {
            singleStepRequested = true;
        }
    }

    bool consumeSingleStepRequest()
    {
        const bool requested = mode == EditorPlayMode::Paused && singleStepRequested;

        singleStepRequested = false;
        return requested;
    }

  private:
    EditorPlayMode mode = EditorPlayMode::Edit;
    bool singleStepRequested = false;
};