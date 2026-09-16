#pragma once

#include <memory>

#include "engineApi.hpp"

class Scene;

class EditorPlaySnapshot final
{
public:
    ENGINE_API EditorPlaySnapshot();
    ENGINE_API ~EditorPlaySnapshot();

    EditorPlaySnapshot(
        const EditorPlaySnapshot&
    ) = delete;

    EditorPlaySnapshot& operator=(
        const EditorPlaySnapshot&
    ) = delete;

    ENGINE_API void capture(
        const Scene& scene
    );

    ENGINE_API void restore(
        Scene& scene
    );

    ENGINE_API void clear();
    ENGINE_API bool isCaptured() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};