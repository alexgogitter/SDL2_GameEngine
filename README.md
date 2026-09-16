# SDL2 Game Engine

A small C++17 game-engine prototype using SDL2 as its platform layer, OpenGL 3.3
for programmable rendering, Box2D for 2D physics, GLM for mathematics, and Dear
ImGui for runtime tooling.

Made by Matthew Bannister and Alex Riddell.

## Current Capabilities

- command-queued 2D rendering
- orthographic 2D and perspective 3D camera classes
- albedo, normal, height, emission, diffuse/roughness, and specular/shiny maps
- TexturePacker XML sprite animation with event-driven transitions
- forward-rendered 2D point lights
- OpenGL texture caching with linear/sRGB handling
- Box2D rigid bodies and box colliders
- object/component composition
- OpenGL-rendered ImGui interface

Shadows and 3D mesh rendering are planned but not yet implemented. See the
[current status](/Documentation/Current_Status.md) for precise feature boundaries.

## Build

The recommended Windows build uses Visual Studio 2022, CMake, and vcpkg. See
[Getting Started](/Documentation/Getting_Started.md) for complete dependency
installation steps.

After installing vcpkg and setting `VCPKG_ROOT`, run from the repository root:

```powershell
git submodule update --init --recursive
cmake --preset msvc-vs2022
cmake --build --preset msvc-debug --target SDL2_GameEngine --parallel
```

Run the Debug build from the repository root:

```powershell
.\build\msvc-vs2022\Debug\SDL2_GameEngine.exe
```

The repository also contains a legacy Makefile workflow for the MSYS2 UCRT64
MinGW toolchain:

```bash
make debug
```

Run:

```powershell
.\bin\SDL2_GameEngine_debug.exe
```

The current Makefile targets the MSYS2 UCRT64 MinGW toolchain on Windows.

## Documentation

Start with the [documentation index](/Documentation/Index.md).

Recommended reading order:

1. [Getting Started](/Documentation/Getting_Started.md)
2. [Engine Architecture](/Documentation/Overview.md)
3. [OpenGL Renderer](/Documentation/OpenGL_Renderer.md)
4. [Objects and Components](/Documentation/Components.md)
5. [Blender Asset Workflow](/Documentation/Blender_Asset_Workflow.md)

Public engine headers use Doxygen-style comments so IntelliSense shows parameter,
return, ownership, unit, and lifecycle documentation while editing C++.

## Repository Layout

```text
src/             engine and demo C++ source
res/             runtime textures, fonts, maps, and GLSL shaders
external/        GLAD, GLM, ImGui, and Box2D dependencies
Documentation/   maintained engine manual
bin/             local build output
dist/            packaged release output
```

## Contribution Expectations

- Use feature branches for major work.
- Reference the relevant issue or project item in commits.
- Prefer RAII and explicit ownership.
- Use four-space indentation.
- Put function opening braces on a new line.
- Build with warnings enabled before review.
- Update both Markdown documentation and public Doxygen comments when behavior or
  APIs change. See [Documentation Maintenance](/Documentation/Documentation_Maintenance.md).
