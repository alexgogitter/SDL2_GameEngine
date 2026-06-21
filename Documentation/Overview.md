# Codebase Overview

This project is a small SDL2-based game engine prototype. The current codebase is organized around a simple frame loop, a renderer wrapper, a resource manager, an immediate-mode UI layer built on ImGui, and a lightweight object/component system.

## High-Level Flow

`main.cpp` owns the application loop. The current flow is:

1. Create the `Renderer` and initialize SDL, SDL_image, and SDL_ttf.
2. Create the `Interface` wrapper for ImGui.
3. Create a `Time` helper for frame timing.
4. Create a `Resource_manager` for texture and font handles.
5. Load a demo texture directly with SDL_image.
6. Run the main loop:
   - clear the frame
   - compute delta time
   - draw the demo texture
   - process SDL events
   - update FPS tracking
   - render ImGui callbacks through `Interface`
   - present the frame

## Subsystems

### Renderer

The `Renderer` class wraps SDL window and renderer creation. It also initializes the graphics-related SDL subsystems used by the project.

Key responsibilities:

- create the window and renderer
- initialize SDL, SDL_image, and SDL_ttf
- expose `SDL_Window*` and `SDL_Renderer*` accessors
- render text via `Renderer_ttf`
- present frames

Current notes:

- `Renderer_Init()` creates the window with the configured title and dimensions.
- `Renderer_ttf()` renders text to a texture and draws it to the supplied destination rectangle.
- `Renderer_Draw(Object*)` currently only presents the frame and does not render the object itself.

### Resource Manager

`Resource_manager` caches textures and fonts behind integer handles.

Texture flow:

- `loadTexture()` loads a texture from a file path and returns a handle.
- `getTexture()` returns the cached texture pointer or reloads it if needed.
- `deleteTexture()` destroys the SDL texture but keeps the handle and file path.

Font flow:

- `loadFont()` loads a font at a specific size and returns a handle.
- `getFont()` returns the cached font pointer or reloads it if needed.
- `deleteFont()` closes the SDL_ttf font but keeps the cached metadata.

Bulk helpers:

- `loadTextures()` preloads a list of textures.
- `loadFonts()` preloads a list of fonts.
- `loadLevelResources()` preloads a texture and font set for a level.

### ImGui Interface

`Interface` owns the ImGui SDL2 and SDLRenderer2 backends.

Responsibilities:

- create and destroy the ImGui context
- forward SDL events into ImGui with `update()`
- register draw callbacks through `addDrawCallback()`
- begin a new ImGui frame and render registered callbacks in `draw()`

The current design lets the rest of the application register UI code without coupling that code directly to the ImGui frame lifecycle.

### Time

`Time` wraps `SDL_GetTicks64()` to provide frame timing values.

It currently exposes:

- `tick()` to mark the start of a timing interval
- `tock()` to compute elapsed time since the last tick
- `getElapsed()` to query the current elapsed duration

The project now uses `std::uint64_t` consistently for timing values.

### FPS Counter

`fpsCounter` is a thin renderer helper that draws a text value, typically the FPS label, using the renderer and a font.

Current behavior:

- opens a default font from `res/fonts/comicz.ttf` in the default constructor
- stores source and destination rectangles for the text draw call
- calls `Renderer::Renderer_ttf()` from `update()`

### Object and Component

`Object` is the base entity container. It owns:

- a texture handle resolved through `Resource_manager`
- a collider rectangle
- child objects
- components

`Object::update()` runs the virtual `Update()` override, then updates all components and children.

`Component` is the base behavior unit attached to an `Object`. It declares a pure virtual `Update()` method and exposes a public `update()` entry point.

### Scene, Camera, Shader, and Map Reader

Several headers are currently present but mostly serve as placeholders or legacy scaffolding:

- `scene.hpp` defines a basic tile struct and a scene container.
- `camera.hpp` contains a commented-out camera implementation.
- `shader.h` contains a commented-out OpenGL shader wrapper.
- `mapReader.h` loads PNG data into a 2D color grid and contains map validation helpers.

## File Map

- `main.cpp`: application entry point and demo loop
- `render.hpp` / `render.cpp`: SDL window, renderer, and text rendering wrapper
- `resource_manager.hpp` / `resource_manager.cpp`: texture and font cache
- `interfaceImplementation.hpp` / `interfaceImplementation.cpp`: ImGui integration
- `time.hpp` / `time.cpp`: frame timing helpers
- `fpsCounter.hpp` / `fpsCounter.cpp`: text-based FPS display helper
- `object.hpp` / `object.cpp`: entity container and update propagation
- `component.hpp` / `component.cpp`: component base class
- `scene.hpp`: scene and tile placeholders
- `mapReader.h`: image-based map loading and validation prototype
- `shader.h`, `camera.hpp`: legacy OpenGL-era scaffolding

## Build And Run

The repository memory notes the primary build command as:

```bash
make
```

The project expects the SDL2, SDL2_image, SDL2_ttf, and ImGui dependencies bundled in the repository structure and linked by the build system.

## Maintenance Notes

- Prefer direct includes for shared core types and APIs instead of relying on transitive includes.
- Avoid global definitions in headers unless they are intentionally `inline` or `constexpr`.
- Keep ownership explicit for SDL objects, especially textures, fonts, and dynamically allocated rectangles.
- Consider moving hardcoded asset paths into a config or resource manifest.
- `mapReader.h` would be safer as a `.cpp` implementation file or a header with `inline` constants and declarations only.