cmake_minimum_required(VERSION 3.25)

project(
    SDL2_GameEngine
    VERSION 0.1.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Match vcpkg's dynamic MSVC runtime.
set(
    CMAKE_MSVC_RUNTIME_LIBRARY
    "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
)

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

find_package(
    unofficial-omniverse-physx-sdk
    CONFIG
    REQUIRED
)

# GLAD
add_library(glad STATIC
    external/glad/src/glad.c
)

target_include_directories(glad
    PUBLIC
        external/glad/include
)

# Dear ImGui
add_library(imgui SHARED
    external/imgui/imgui.cpp
    external/imgui/imgui_demo.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_sdl2.cpp
)

    if(WIN32)
        set_target_properties(imgui PROPERTIES
            WINDOWS_EXPORT_ALL_SYMBOLS ON
        )
    endif()

target_include_directories(imgui
    PUBLIC
        external/imgui
        external/imgui/backends
)

target_link_libraries(imgui
    PUBLIC
        SDL2::SDL2
)

# Existing Box2D submodule
set(BOX2D_SAMPLES OFF CACHE BOOL "" FORCE)
set(BOX2D_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BOX2D_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(BOX2D_DOCS OFF CACHE BOOL "" FORCE)

add_subdirectory(external/box2d)

# Current engine executable
# Core engine runtime
set(ENGINE_RUNTIME_SOURCES
    src/animationControllerComponent.cpp
    src/camera2D.cpp
    src/camera3D.cpp
    src/colliderComponent.cpp
    src/component.cpp
    src/eventListener.cpp
    src/fpsCounter.cpp
    src/interfaceImplementation.cpp
    src/light2DComponent.cpp
    src/object.cpp
    src/physicsWorld2D.cpp
    src/render2D.cpp
    src/render3D.cpp
    src/window.cpp
    src/resource_manager.cpp
    src/rigidbodyComponent.cpp
    src/shader.cpp
    src/spriteRendererComponent.cpp
    src/stb_image.cpp
    src/textureComponent.cpp
    src/time.cpp
    src/objectId.hpp
    src/objectId.cpp
)

add_library(EngineRuntime SHARED
    ${ENGINE_RUNTIME_SOURCES}
    src/layer.hpp
    src/layerRegistry.hpp
    src/layerRegistry.cpp
    src/scene.cpp
    src/componentRegistry.cpp
    src/builtInComponents.hpp
    src/builtinComponents.cpp
    src/componentProperty.hpp
    src/componentInspector.hpp
    src/componentInspector.cpp
    src/editorSelection.hpp
    src/editorSelection.cpp
    src/objectInspector.hpp
    src/objectInspector.cpp
    src/sceneHierarchy.hpp
    src/sceneHierarchy.cpp
    src/physxContext.hpp
    src/physxContext.cpp
    src/transform3D.hpp
    src/mesh3D.hpp
    src/meshFilterComponent.hpp
    src/meshFilter.cpp
    src/cameraComponent.hpp
    src/cameraComponent.cpp
    src/cameraSystem.hpp
    src/cameraSystem.cpp
    src/cameraOutputPanel.hpp
    src/cameraOutputPanel.cpp
    src/renderTarget.hpp
    src/renderTarget.cpp
    src/gameViewPanel.hpp
    src/gameViewPanel.cpp
    src/editorCamera.hpp
    src/editorCamera.cpp
    src/sceneViewPanel.hpp
    src/sceneViewPanel.cpp
    src/editorDockspace.hpp
    src/editorDockspace.cpp
    src/gameViewportInput.hpp
    src/gameViewInput.hpp
    src/meshRendererComponent.hpp
    src/meshRendererComponent.cpp
    src/physicsWorld3D.hpp
    src/physicsWorld3D.cpp
    src/rigidbody3DComponent.hpp
    src/rigidbody3DComponent.cpp
    src/editorPlayState.hpp
    src/editorPlayToolbar.hpp
    src/editorPlayToolbar.cpp
    src/editorPlaySnapshot.hpp
    src/editorPlaySnapshot.cpp
    src/boxCollider3DComponent.hpp
    src/boxCollider3DComponent.cpp
    src/editorGizmoOverlay.hpp
    src/editorGizmoOverlay.cpp
    src/lighting3D.hpp
    src/lighting3D.cpp
    src/window.hpp
)

add_library(Engine::Runtime ALIAS EngineRuntime)

target_include_directories(EngineRuntime
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/external"
)

target_compile_definitions(EngineRuntime
    PRIVATE
        ENGINE_RUNTIME_BUILD
)

# These are PUBLIC temporarily because several current engine headers expose
# SDL, ImGui, Box2D and GLM types. We will reduce that leakage later.
target_link_libraries(EngineRuntime
    PUBLIC
        SDL2::SDL2
        SDL2_image::SDL2_image
        SDL2_ttf::SDL2_ttf
        OpenGL::GL
        box2d::box2d
        glad
        imgui
)

target_link_libraries(EngineRuntime
    PRIVATE
        unofficial::omniverse-physx-sdk::sdk
)

# Optional sample game. It is compiled separately and
# is not linked into the editor.
add_library(ConstructionSampleModule STATIC
    samples/construction/constructionSample.cpp
    samples/construction/ConstructionSample.hpp
)

target_include_directories(
    ConstructionSampleModule
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/samples/construction"
)

target_link_libraries(
    ConstructionSampleModule
    PRIVATE
        Engine::Runtime
)

# Generic engine editor.
add_executable(SDL2_GameEngine
    src/main.cpp
)

target_link_libraries(SDL2_GameEngine
    PRIVATE
        SDL2::SDL2main
        Engine::Runtime
)

if(MSVC)
    target_compile_options(EngineRuntime PRIVATE
        /W4
        /Zc:__cplusplus
    )

    target_compile_options(SDL2_GameEngine PRIVATE
        /W4
        /Zc:__cplusplus
    )

    target_compile_options(ConstructionSampleModule PRIVATE
        /W4
        /Zc:__cplusplus
)
endif()

# Copy SDL and its transitive runtime DLLs beside the executable.
if(WIN32)
    add_custom_command(
        TARGET SDL2_GameEngine
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:SDL2_GameEngine>
            $<TARGET_FILE_DIR:SDL2_GameEngine>
        COMMAND_EXPAND_LISTS
    )

    set(
    PHYSX_RUNTIME_DIRECTORY
    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/$<$<CONFIG:Debug>:debug/>bin"
    )

    add_custom_command(
        TARGET SDL2_GameEngine
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PHYSX_RUNTIME_DIRECTORY}/PhysX_64.dll"
            "${PHYSX_RUNTIME_DIRECTORY}/PhysXCommon_64.dll"
            "${PHYSX_RUNTIME_DIRECTORY}/PhysXFoundation_64.dll"
            "${PHYSX_RUNTIME_DIRECTORY}/PhysXCooking_64.dll"
            $<TARGET_FILE_DIR:SDL2_GameEngine>
        VERBATIM
    )
endif()

set_target_properties(SDL2_GameEngine PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

set_property(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    PROPERTY VS_STARTUP_PROJECT SDL2_GameEngine
)