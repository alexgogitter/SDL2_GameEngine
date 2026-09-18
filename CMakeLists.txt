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
find_package(nlohmann_json CONFIG REQUIRED)

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
    src/EngineCore/sceneSerialization.cpp
    src/EngineCore/scriptModule.cpp
    src/EngineCore/logger.cpp
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
    src/Editor/editorMenuToolbar.cpp
    src/Editor/editorProject.cpp
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
        ENGINE_SOURCE_DIRECTORY="${CMAKE_SOURCE_DIR}"
        ENGINE_CMAKE_COMMAND="${CMAKE_COMMAND}"
        ENGINE_BUILD_DIRECTORY="${CMAKE_BINARY_DIR}"
        ENGINE_BUILD_CONFIG="$<CONFIG>"
        ENGINE_SCRIPT_PATH="${CMAKE_BINARY_DIR}/scripts/$<CONFIG>/UserScripts${CMAKE_SHARED_LIBRARY_SUFFIX}"
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
        nlohmann_json::nlohmann_json
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

# User gameplay code is a separate DLL. Only shadow copies are loaded by the editor.
file(GLOB USER_SCRIPT_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/scripts/*.cpp")

# Script compilation runs while the editor has EngineRuntime.dll loaded.  Linking
# UserScripts against the EngineRuntime *target* makes MSBuild build that target
# first, which attempts to overwrite the loaded DLL whenever an engine/editor
# source file is newer.  Scripts only need the import library produced by the
# last editor build, so expose it as an imported binary instead.  Imported
# targets have no build dependency: a normal editor build refreshes this API
# library, while a script-only build never relinks EngineRuntime.dll.
add_library(EngineScriptApi SHARED IMPORTED GLOBAL)
set_target_properties(EngineScriptApi PROPERTIES
    IMPORTED_CONFIGURATIONS "DEBUG;RELEASE;RELWITHDEBINFO;MINSIZEREL"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/src;${CMAKE_CURRENT_SOURCE_DIR}/src/EngineCore;${CMAKE_CURRENT_SOURCE_DIR}/external;${CMAKE_CURRENT_SOURCE_DIR}/external/glm;${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include;${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/SDL2")
foreach(ENGINE_SCRIPT_CONFIG Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER "${ENGINE_SCRIPT_CONFIG}" ENGINE_SCRIPT_CONFIG_UPPER)
    set_property(TARGET EngineScriptApi PROPERTY
        "IMPORTED_IMPLIB_${ENGINE_SCRIPT_CONFIG_UPPER}"
        "${CMAKE_BINARY_DIR}/${ENGINE_SCRIPT_CONFIG}/EngineRuntime.lib")
endforeach()

add_library(UserScripts SHARED
    src/ScriptHost/scriptModuleExports.cpp
    ${USER_SCRIPT_SOURCES}
)
target_link_libraries(UserScripts PRIVATE EngineScriptApi)
set_target_properties(UserScripts PROPERTIES
    PREFIX ""
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/scripts/$<CONFIG>"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/scripts/$<CONFIG>")
target_compile_definitions(SDL2_GameEngine PRIVATE
    ENGINE_PROJECT_DIRECTORY="${CMAKE_SOURCE_DIR}"
    ENGINE_SCRIPT_PATH="$<TARGET_FILE:UserScripts>")

# A full editor build produces EngineRuntime.lib first, then refreshes scripts.
# Do not model this as a target dependency: the editor's in-process script build
# must be able to build UserScripts without asking MSBuild to rebuild its loaded
# EngineRuntime.dll.
add_custom_command(
    TARGET SDL2_GameEngine
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} --build "${CMAKE_BINARY_DIR}" --config "$<CONFIG>" --target UserScripts --parallel
    COMMENT "Building user scripts against the completed engine API"
    VERBATIM)

add_executable(GamePlayer src/Player/gamePlayer.cpp)
target_link_libraries(GamePlayer PRIVATE SDL2::SDL2main Engine::Runtime)
set_target_properties(GamePlayer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/dist/Game/$<CONFIG>"
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:GamePlayer>")

# Stage content both for direct player builds and explicit content-only refreshes.
set(STAGE_GAME_COMMAND ${CMAKE_COMMAND}
    "-DGAME_DIR=$<TARGET_FILE_DIR:GamePlayer>"
    "-DRUNTIME=$<TARGET_FILE:EngineRuntime>"
    "-DIMGUI=$<TARGET_FILE:imgui>"
    "-DSCRIPTS=$<TARGET_FILE:UserScripts>"
    "-DSOURCE_DIR=${CMAKE_SOURCE_DIR}"
    "-DVCPKG_BIN=${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/$<$<CONFIG:Debug>:debug/>bin"
    "-DVCPKG_SHARE=${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share"
    "-DCONFIG=$<CONFIG>"
    -P "${CMAKE_SOURCE_DIR}/cmake/StageGame.cmake")
add_custom_command(TARGET GamePlayer POST_BUILD COMMAND ${STAGE_GAME_COMMAND} VERBATIM)
add_custom_target(StageGame ALL
    COMMAND ${CMAKE_COMMAND} --build "${CMAKE_BINARY_DIR}" --config "$<CONFIG>" --target UserScripts --parallel
    COMMAND ${STAGE_GAME_COMMAND}
    DEPENDS GamePlayer
    VERBATIM)

include(CTest)
if(BUILD_TESTING)
    add_library(ReloadScriptFixture SHARED tests/reloadScripts.cpp)
    add_library(IncompatibleScriptFixture SHARED tests/reloadScripts.cpp)
    target_compile_definitions(IncompatibleScriptFixture PRIVATE INCOMPATIBLE_SCRIPT_FIXTURE)
    foreach(FIXTURE ReloadScriptFixture IncompatibleScriptFixture)
        target_link_libraries(${FIXTURE} PRIVATE Engine::Runtime)
        set_target_properties(${FIXTURE} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test-modules/$<CONFIG>")
    endforeach()
    add_executable(EngineFoundationTests tests/engineFoundationTests.cpp)
    target_link_libraries(EngineFoundationTests PRIVATE SDL2::SDL2main Engine::Runtime)
    target_compile_definitions(EngineFoundationTests PRIVATE
        ENGINE_SCRIPT_PATH="$<TARGET_FILE:UserScripts>"
        RELOAD_FIXTURE_PATH="$<TARGET_FILE:ReloadScriptFixture>"
        INCOMPATIBLE_FIXTURE_PATH="$<TARGET_FILE:IncompatibleScriptFixture>")
    add_dependencies(EngineFoundationTests UserScripts ReloadScriptFixture IncompatibleScriptFixture)
    add_test(NAME EngineFoundations COMMAND EngineFoundationTests)
    set_tests_properties(EngineFoundations PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif()
