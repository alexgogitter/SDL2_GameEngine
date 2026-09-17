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

# Core engine runtime sources
set(ENGINE_RUNTIME_SOURCES
    src/EngineCore/animationControllerComponent.cpp
    src/EngineCore/boxCollider3DComponent.cpp
    src/EngineCore/builtinComponents.cpp
    src/EngineCore/camera2D.cpp
    src/EngineCore/camera3D.cpp
    src/EngineCore/cameraComponent.cpp
    src/EngineCore/cameraSystem.cpp
    src/EngineCore/colliderComponent.cpp
    src/EngineCore/component.cpp
    src/EngineCore/componentRegistry.cpp
    src/EngineCore/eventListener.cpp
    src/EngineCore/fpsCounter.cpp
    src/EngineCore/light2DComponent.cpp
    src/EngineCore/lighting3D.cpp
    src/EngineCore/layerRegistry.cpp
    src/EngineCore/meshFilter.cpp
    src/EngineCore/meshRendererComponent.cpp
    src/EngineCore/object.cpp
    src/EngineCore/objectId.cpp
    src/EngineCore/physicsWorld2D.cpp
    src/EngineCore/physicsWorld3D.cpp
    src/EngineCore/physxContext.cpp
    src/EngineCore/render2D.cpp
    src/EngineCore/render3D.cpp
    src/EngineCore/renderTarget.cpp
    src/EngineCore/resource_manager.cpp
    src/EngineCore/rigidbody3DComponent.cpp
    src/EngineCore/rigidbodyComponent.cpp
    src/EngineCore/scene.cpp
    src/EngineCore/shader.cpp
    src/EngineCore/spriteRendererComponent.cpp
    src/EngineCore/stb_image.cpp
    src/EngineCore/textureComponent.cpp
    src/EngineCore/time.cpp
    src/EngineCore/window.cpp
)

set(ENGINE_EDITOR_SOURCES
    src/Editor/cameraOutputPanel.cpp
    src/Editor/componentInspector.cpp
    src/Editor/editorCamera.cpp
    src/Editor/editorDockspace.cpp
    src/Editor/editorGizmoOverlay.cpp
    src/Editor/editorPlaySnapshot.cpp
    src/Editor/editorPlayToolbar.cpp
    src/Editor/editorSelection.cpp
    src/Editor/gameViewPanel.cpp
    src/Editor/interfaceImplementation.cpp
    src/Editor/objectInspector.cpp
    src/Editor/sceneHierarchy.cpp
    src/Editor/sceneViewPanel.cpp
)

file(GLOB ENGINE_RUNTIME_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/EngineCore/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/EngineCore/*.hpp"
)

file(GLOB ENGINE_EDITOR_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Editor/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Editor/*.hpp"
)

add_library(EngineRuntime SHARED
    ${ENGINE_RUNTIME_SOURCES}
    ${ENGINE_EDITOR_SOURCES}
    ${ENGINE_RUNTIME_HEADERS}
    ${ENGINE_EDITOR_HEADERS}
)

add_library(Engine::Runtime ALIAS EngineRuntime)

target_include_directories(EngineRuntime
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/Editor"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/EngineCore"
        "${CMAKE_CURRENT_SOURCE_DIR}/external"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/glm"
)

target_compile_definitions(EngineRuntime
    PRIVATE
        ENGINE_RUNTIME_BUILD
)

target_link_libraries(EngineRuntime
    PUBLIC
        SDL2::SDL2
        SDL2_image::SDL2_image
        SDL2_ttf::SDL2_ttf
        OpenGL::GL
        box2d
        glad
        imgui
    PRIVATE
        unofficial::omniverse-physx-sdk::sdk
)

# Generic engine editor
add_executable(SDL2_GameEngine
    src/Editor/editorEntryPoint.cpp
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
endif()

# Copy SDL and PhysX runtime DLLs beside the executable.
if(WIN32)
    add_custom_command(
        TARGET SDL2_GameEngine
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:SDL2_GameEngine>
            $<TARGET_FILE_DIR:SDL2_GameEngine>
        COMMAND_EXPAND_LISTS
    )

    if(DEFINED VCPKG_TARGET_TRIPLET)
        set(PHYSX_RUNTIME_DIRECTORY
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
endif()

set_target_properties(SDL2_GameEngine PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

set_property(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    PROPERTY VS_STARTUP_PROJECT SDL2_GameEngine
)
