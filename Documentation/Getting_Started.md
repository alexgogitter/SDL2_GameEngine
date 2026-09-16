# Getting Started

This guide covers the supported Windows development workflow for the current
SDL2, OpenGL, and PhysX engine.

## Requirements

The primary build uses Visual Studio 2022, CMake, and vcpkg.

Required tools and libraries:

- Git, including Git submodule support
- Visual Studio 2022 with the **Desktop development with C++** workload
- A Windows 10 or 11 SDK installed through Visual Studio
- CMake 3.25 or newer
- vcpkg
- OpenGL 3.3-capable graphics drivers

The vcpkg manifest installs these binary dependencies:

- SDL2
- SDL2_image
- SDL2_ttf
- NVIDIA PhysX

The following source dependencies are included in the repository or initialized
as a submodule:

- GLAD under `external/glad`
- GLM under `external/glm`
- Dear ImGui under `external/imgui`
- Box2D under `external/box2d`

## First-Time Setup

### 1. Clone the repository

From PowerShell:

```powershell
git clone --recurse-submodules <repository-url>
cd SDL2_GameEngine
```

For an existing clone, initialize the Box2D submodule with:

```powershell
git submodule update --init --recursive
```

### 2. Install vcpkg

Install vcpkg outside the repository, then bootstrap it:

```powershell
git clone https://github.com/microsoft/vcpkg.git C:\dev\vcpkg
cd C:\dev\vcpkg
.\bootstrap-vcpkg.bat
```

Set `VCPKG_ROOT` to that directory. Restart PowerShell and VS Code after using
`setx` so the environment variable is available to CMake:

```powershell
setx VCPKG_ROOT C:\dev\vcpkg
```

### 3. Configure dependencies and Visual Studio

Return to the repository root and configure the MSVC preset:

```powershell
cd C:\Projects\SDL2_GameEngine
cmake --preset msvc-vs2022
```

The vcpkg manifest is read during configuration. It installs the packages from
`vcpkg.json` for the `x64-windows` triplet, including the PhysX package required
by `find_package(unofficial-omniverse-physx-sdk)`.

### 4. Build

Build the Debug target with:

```powershell
cmake --build --preset msvc-debug `
    --target SDL2_GameEngine `
    --parallel
```

For a Release build:

```powershell
cmake --build --preset msvc-release `
    --target SDL2_GameEngine `
    --parallel
```

## Build (Legacy Make Workflow)

The repository also contains an older Makefile for the MSYS2 UCRT64 MinGW
toolchain. Use it only if that toolchain is already installed and you are not
working with the MSVC/CMake build above.

Debug build:

```bash
make debug
```

Release build:

```bash
make release
```

Portable package:

```bash
make package
```

The legacy workflow produces:

```text
bin/SDL2_GameEngine_debug.exe
bin/SDL2_GameEngine.exe
```

The recommended MSVC build produces:

```text
build/msvc-vs2022/Debug/SDL2_GameEngine.exe
build/msvc-vs2022/Release/SDL2_GameEngine.exe
```

If Box2D needs to be regenerated:

```bash
make clean-box2d
make box2d
```

## Run

Run the recommended Debug executable with the repository root as its working
directory:

```powershell
.\build\msvc-vs2022\Debug\SDL2_GameEngine.exe
```

The runtime expects the following DLLs to be discoverable beside the executable
or on `PATH`:

- `SDL2.dll`
- `SDL2_image.dll`
- `SDL2_ttf.dll`
- PhysX runtime DLLs

The CMake post-build step copies these DLLs beside the executable. If they are
missing, rebuild after confirming that `VCPKG_ROOT` is set and the MSVC preset
was configured successfully.

## Demo Controls

- Left-click in the world to create a dynamic Box2D square.
- Clicking over ImGui does not create a square.
- The blue player carries a warm `Light2DComponent`.
- Spawned squares use the stone albedo texture.

## Minimal Application Lifecycle

The order here is important because OpenGL resources must be destroyed while the
context is still valid.

```cpp
Renderer renderer(1920, 1080, 60, IMG_INIT_PNG, "My Game");
if (renderer.Renderer_Init() != 0)
{
    return 1;
}

Interface* interface = Interface::create(
    renderer.get_SDLWindow(),
    renderer.get_GLContext()
);

{
    Resource_manager resources;
    Camera2D camera(1920.0f, 1080.0f);
    renderer.SetActiveCamera(&camera);

    // Create and run the scene here.
}

delete interface;
renderer.Renderer_Close();
```

The resource manager must be constructed after `Renderer_Init()` and destroyed
before `Renderer_Close()`.

## Minimal Frame

```cpp
renderer.BeginFrame({0.05f, 0.06f, 0.08f, 1.0f});

for (const std::unique_ptr<Object>& object : gameObjects)
{
    object->update(deltaTimeMilliseconds);
    object->draw(&renderer);
}

renderer.Render2D();
interface->draw();
renderer.Renderer_PresentFrame();
```

`Object::draw()` and component draw methods submit commands. They do not render
immediately. `Renderer::Render2D()` executes the completed queue.

## First Sprite

```cpp
auto square = std::make_unique<PhysicsBoxObject>(
    "Crate",
    resources,
    &renderer,
    physicsWorld,
    glm::vec2(400.0f, 200.0f),
    glm::vec2(64.0f, 64.0f),
    BodyType2D::Dynamic,
    glm::vec4(255.0f),
    false
);

auto* sprite = new SpriteRendererComponent(
    square.get(),
    resources,
    "res/textures/crate/crate_albedo.png"
);
sprite->setDrawSize({64.0f, 64.0f});
square->add_Component(sprite);
```

Once passed to `add_Component()`, the component is owned and eventually deleted
by the object. The physics box size and sprite draw size are independent; the
example simply makes them match.

## Animation Test Controls

The current demo scene includes a TexturePacker atlas test drone. Press number
keys `1` through `7` to switch its rendered angle:

```text
1: 0 degrees
2: 15 degrees
3: 30 degrees
4: 45 degrees
5: 60 degrees
6: 75 degrees
7: 90 degrees
```

## Common Startup Failures

### Shader file cannot be opened

Run the executable from the repository root. The current shader paths begin with
`res/shaders/`.

### OpenGL context creation fails

Update the graphics driver and confirm that the machine supports OpenGL 3.3.

### Executable cannot be replaced during build

Close the running game before rebuilding. Windows locks an executable while it
is running.

For more diagnostics, see [Troubleshooting](/Documentation/Troubleshooting.md).
