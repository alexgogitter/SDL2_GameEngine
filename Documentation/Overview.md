# Engine Architecture Overview

The project is an SDL2 game-engine prototype with an OpenGL 3.3 graphics
backend, Box2D physics, Dear ImGui tooling, cached resources, and an
object/component model.

SDL2 owns platform concerns. OpenGL owns rendering.

## System Map

```text
SDL2
  window, events, timing, image/font decoding
       |
       v
Renderer ---------------- Camera
  OpenGL context           Camera2D / Camera3D
  shaders
  render queues
  lights
       ^
       |
Object ---- Transform2D <---- RigidbodyComponent <---- Box2D
  |
  +-- ColliderComponent
  +-- SpriteRendererComponent ---- Resource_manager ---- OpenGL textures
  +-- AnimationControllerComponent
  +-- Light2DComponent
  +-- child Objects

ImGui Interface ---- SDL events + final OpenGL UI pass
```

## Application Ownership

The current demo owns engine systems directly in `main.cpp`.

```text
Renderer
  Interface
  Resource_manager
  PhysicsWorld2D
    gameObjects
      Components
  Camera2D
```

Destruction order matters:

1. game objects destroy Box2D bodies and components
2. physics world is destroyed
3. resource manager deletes OpenGL textures and fonts
4. ImGui deletes its OpenGL resources
5. renderer deletes its GPU buffers, context, and window

The actual declarations use nested scope and declaration order to achieve this.

## Main Loop

The current loop follows:

```text
start frame timer
process SDL events
handle gameplay input
step Box2D fixed simulation
begin renderer frame
update objects and components
submit object/component drawing
execute 2D render queue
draw ImGui
swap OpenGL buffers
record frame duration
```

Physics currently steps before object components synchronize transforms. That
ensures rendering uses the latest completed Box2D state.

## Coordinate and Unit Conventions

### Rendering

- 2D world X: right
- 2D world Y: down
- position unit: logical pixels
- rotation unit: radians
- `Transform2D::scale`: submitted-quad half-size for legacy/fallback draws
- sprite draw rect: local visual rectangle owned by `SpriteRendererComponent`
- collider rect: full physics size owned by `ColliderComponent`
- material colours: linear 0..1
- legacy `Object::draw_colour`: byte-style 0..255 values

### Physics

- Box2D operates in metres
- `PhysicsWorld2D::PixelsPerMetre` is `100.0`
- rigid-body helpers expose pixel-friendly APIs and convert internally

### Lighting

- point-light X/Y: 2D world pixels
- point-light Z: pseudo-height above the sprite plane
- radius: planar world pixels
- normals: tangent-space OpenGL convention

## Rendering Architecture

Rendering is deferred at the command level, although the current lighting shader
is a forward shader.

`Object::draw()` does not immediately draw. It asks the renderer to store command
data. Once the complete scene has submitted, `Render2D()` can sort commands and
shade them using every light collected during the frame.

This distinction matters:

- command-deferred: CPU submissions are collected before GPU execution
- deferred lighting: geometry first writes a G-buffer, then lights shade it

The engine currently uses the first and may later add the second.

## Resource Architecture

Gameplay code stores stable `TextureHandle` values. `Resource_manager` maps those
handles to OpenGL texture objects and preserves file metadata for reload.

Material components resolve handles into `Material2DRenderState` only while
submitting. This keeps material configuration separate from backend GPU IDs.

## Component Architecture

Components combine behavior through composition:

- `RigidbodyComponent`: owns and synchronizes a Box2D body
- `ColliderComponent`: owns physics shape size/configuration attached to a rigid body
- `SpriteRendererComponent`: owns 2D material configuration and visual draw rect
- `AnimationControllerComponent`: updates sprite atlas frames and state transitions
- `Light2DComponent`: owns a point-light configuration

`PhysicsBoxObject` is a convenience type that creates rigid-body and collider
components together.

Components and children use ownership transfer into `Object`. See
[Components](/Documentation/Components.md) before adding or removing them.

## Current Rendering Model

The shader supports:

- untextured colour quads
- albedo and alpha
- diffuse/roughness response
- tangent-space normal maps
- pseudo-height maps
- emission maps
- specular/shiny response
- ambient light
- up to sixteen point lights
- unlit materials
- stable integer render layers

It does not yet support shadows, metalness, a physically based BRDF, HDR,
post-processing, sprite-sheet source rectangles, or 3D meshes.

## Camera Architecture

`Renderer` depends on the abstract `Camera` interface. `Camera2D` supplies the
current orthographic projection and screen-to-world conversion. `Camera3D`
supplies valid perspective matrices for a future mesh pass.

The renderer holds a non-owning pointer to one active camera.

## ImGui Architecture

The `Interface` singleton owns the ImGui context and GPU resources. SDL events
go through the official SDL2 platform backend. The engine uses a small custom
OpenGL renderer for ImGui draw lists.

ImGui is drawn after the world, so UI does not participate in scene lighting.

## Source Map

| Area | Main files |
|---|---|
| Window and OpenGL renderer | `render.hpp`, `render.cpp` |
| Shader wrapper | `shader.hpp`, `shader.cpp` |
| Sprite GLSL | `res/shaders/sprite2d.vert`, `sprite2d.frag` |
| Cameras | `camera.hpp`, `camera2D.*`, `camera3D.*` |
| Materials and lights | `material2D.hpp`, `lighting2D.hpp` |
| Render components | `spriteRendererComponent.*`, `light2DComponent.*` |
| Resource cache | `resource_manager.*` |
| Entity composition | `object.*`, `component.*`, `transform.hpp` |
| Physics | `physicsWorld2D.*`, `rigidbodyComponent.*`, `colliderComponent.*` |
| ImGui | `interfaceImplementation.*` |
| Demo loop | `main.cpp` |

## Design Direction

Preserve these boundaries as the engine grows:

- SDL remains the platform layer.
- Components describe scene intent and submit data.
- Renderer passes own API-specific GPU work.
- Resource handles hide GPU object names from gameplay code.
- 2D and 3D use separate transforms, materials and queues.
- Blender workflows produce versioned runtime assets, not direct `.blend` loads.
- Documentation and IntelliSense comments change with the APIs they describe.
