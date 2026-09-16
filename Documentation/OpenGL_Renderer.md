# OpenGL Renderer

The engine uses SDL2 for window and platform management and OpenGL 3.3 core for
all world and UI rendering. GLAD loads OpenGL functions after SDL creates and
makes the context current.

## Initialization

`Renderer::Renderer_Init()` performs these stages:

1. initialize SDL video
2. initialize SDL_image with configured flags
3. initialize SDL_ttf
4. request OpenGL 3.3 core, double buffering, depth, stencil and sRGB capability
5. create a resizable, high-DPI SDL OpenGL window
6. create and make current the SDL GL context
7. load OpenGL entry points through GLAD
8. enable vertical synchronization
9. compile the sprite shaders
10. create the shared quad VAO, VBO and index buffer

Initialization returns zero on success and a non-zero stage-specific code on
failure.

## Context Ownership

`Renderer` owns:

- `SDL_Window*`
- `SDL_GLContext`
- shared 2D quad GPU buffers
- sprite shader program
- per-frame render commands
- per-frame point lights

It does not own the active camera.

All OpenGL resource owners must be destroyed before `Renderer_Close()`. In the
current application, `Resource_manager` and scene objects live in a nested scope,
then ImGui is deleted, then the renderer closes.

## Why SDL_Renderer Is Not Used

Do not create an `SDL_Renderer` for the OpenGL window and do not call
`SDL_RenderCopy`, `SDL_RenderGeometry`, or `SDL_RenderPresent`.

Those APIs belong to SDL's high-level renderer and do not expose the programmable
shader pipeline needed by normal maps, height maps, custom light accumulation,
shadow passes, or 3D meshes.

## Frame Lifecycle

```cpp
renderer.BeginFrame(clearColour);

for (const auto& object : gameObjects)
{
    object->update(deltaMilliseconds);
    object->draw(&renderer);
}

renderer.Render2D();
interface->draw();
renderer.Renderer_PresentFrame();
```

### BeginFrame

`BeginFrame()`:

- asks SDL for drawable and logical window sizes
- updates the active camera viewport
- sets the OpenGL viewport to drawable pixel dimensions
- enables framebuffer sRGB conversion for the world pass
- clears colour, depth and stencil buffers
- clears 2D render commands and light submissions
- resets stable submission numbering

Submit nothing before `BeginFrame()`, because it clears the queues.

### Submission

Objects and components call:

```cpp
SubmitSolidQuad2D(...);
SubmitSprite2D(...);
SubmitOutline2D(...);
SubmitLight2D(...);
```

These functions copy lightweight command data into CPU vectors. No geometry is
drawn at submission time.

This design is required for:

- stable ordering
- future batching
- collecting lights before shading any sprite
- multi-pass shadows and deferred rendering
- scene culling
- editor inspection of render queues

### Render2D

`Render2D()` stable-sorts commands first by `renderLayer`, then by submission
order. It configures alpha blending, disables depth/culling for the current 2D
pass, uploads the camera matrix and light array, and draws each command with the
shared quad mesh.

The current pass binds:

| Texture unit | Map |
|---:|---|
| 0 | albedo |
| 1 | normal |
| 2 | height |
| 3 | emission |
| 4 | diffuse/roughness |
| 5 | specular/shiny |
| 6 | alpha mask |

Transient TTF text textures are deleted immediately after their commands draw.

### ImGui

ImGui renders after the world. Its platform integration uses
`imgui_impl_sdl2`, while its graphics pass is implemented directly with OpenGL.

The UI pass disables framebuffer sRGB conversion because ImGui vertex colours
are authored for direct display. It streams ImGui vertices and indices into
dedicated buffers and uses scissor rectangles for clipping.

### Presentation

`Renderer_PresentFrame()` calls `SDL_GL_SwapWindow()`. Vertical synchronization
is requested during initialization. The stored `frameCap` does not currently
perform explicit CPU frame limiting.

## Quad Geometry

Every 2D sprite uses one unit quad centred at the origin:

```text
(-0.5,-0.5) ---- (0.5,-0.5)
     |                 |
     |                 |
(-0.5, 0.5) ---- (0.5, 0.5)
```

The model matrix applies:

1. world translation
2. Z-axis rotation
3. full size derived from `Transform2D::scale * 2`

This is why renderer-submitted transforms use half-size. A
`SpriteRendererComponent` builds its submitted transform from its own local draw
rect, so sprite visuals can differ from collider size. Debug collider outlines
build their submitted transform from `ColliderComponent::getSizePixels()`.

## Sprite Shader Interface

Vertex attributes:

```text
location 0: vec2 local position
location 1: vec2 texture coordinate
```

Major uniforms:

```text
uModel
uViewProjection
uTint
uSpriteRotation
uNormalStrength
uHeightScale
uLit
uSpecularStrength
uShininess
uAlbedoUvRect / uAlphaMaskUvRect / per-map UV rects
uHasAlbedo / uHasAlphaMask / uHasDiffuse / uHasNormal / uHasHeight / uHasEmission / uHasSpecular
uAmbientLight
uLightCount
uLights[16]
```

Shader sources live in:

```text
res/shaders/sprite2d.vert
res/shaders/sprite2d.frag
```

They are runtime assets and must be included in packages.

## Colour Management

The world renderer performs lighting in linear space:

1. albedo/emission textures use sRGB internal formats
2. OpenGL converts sampled colour into linear values
3. shaders perform lighting in linear space
4. the sRGB framebuffer converts final linear output for display

Alpha mask, normal, height, diffuse/roughness and specular/shiny textures use
ordinary linear RGBA storage.

Clear colours and material tint values supplied directly to the shader should be
treated as linear values in the 0..1 range. Legacy object draw colours are stored
as 0..255 values and normalized during submission.

## High-DPI Behavior

OpenGL viewport size uses `SDL_GL_GetDrawableSize()`. Camera projection and input
use logical `SDL_GetWindowSize()` dimensions. This allows the GPU framebuffer to
use native high-DPI pixels while world coordinates remain tied to logical window
coordinates.

## Render Layers and Transparency

Render layers are integer sort keys. Larger layers render later. Within one
layer, submission order is preserved.

This is sufficient for the current opaque/cutout sprites but is not a complete
transparency solution. A future renderer should separate opaque, alpha-tested,
and blended queues and sort blended sprites back-to-front where necessary.

## Direct OpenGL Rules

- Scene components must submit renderer commands rather than change GL state.
- Resource upload belongs in resource systems, not gameplay objects.
- New passes must leave explicit state for the following pass or use a state
  abstraction.
- GL object destruction must happen with a valid current context.
- Shader paths and bindings must be documented when changed.

## Next Renderer Abstractions

Before adding many passes, introduce:

- framebuffer/render-target wrapper
- texture format and sampler descriptions
- explicit render pass boundaries
- OpenGL debug callback
- GPU profiling labels
- sprite batching or instancing
- spatial visibility and light culling

See [Materials and Lighting](/Documentation/Materials_and_Lighting.md) for the
lighting model and [Extending to 3D](/Documentation/Extending_to_3D.md) for the
future mesh architecture.
