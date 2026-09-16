# SDL Renderer to OpenGL Migration

This document records the architectural change from SDL's high-level 2D
renderer to a programmable OpenGL backend.

## Reason

`SDL_Renderer` supported textured geometry and rotation but did not provide the
custom vertex and fragment shader control required for normal maps, height-aware
lighting, shadow maps, deferred rendering, or a future 3D workflow.

SDL2 remains valuable as the platform layer. Only graphics ownership changed.

## Before

```text
SDL_Window
  -> SDL_Renderer
      -> SDL_Texture
      -> SDL_RenderGeometry
      -> ImGui SDLRenderer backend
```

Components performed immediate rendering during `Object::draw()`.

## After

```text
SDL_Window + SDL_GLContext
  -> GLAD-loaded OpenGL 3.3 core
      -> Resource_manager OpenGL textures
      -> queued 2D render commands
      -> GLSL sprite/material lighting
      -> custom OpenGL ImGui renderer
```

Components now submit world-space data. `Renderer::Render2D()` owns execution,
sorting, GPU state and light upload.

## Compatibility Decisions

- `TextureComponent` remains an alias for `SpriteRendererComponent`.
- The current pixel-space, positive-Y-down 2D world is preserved.
- Box2D continues using metres internally and converts to pixels.
- `Transform2D::scale` remains half-size for renderer-command compatibility.
  `SpriteRendererComponent` now builds that submitted half-size from its own
  draw rect, while `ColliderComponent` size remains the physics/debug shape.
- SDL_image and SDL_ttf remain in use.

## New Dependencies

GLAD source and headers are vendored under `external/glad`. Windows builds link
against `opengl32`.

Dear ImGui continues using its SDL2 platform backend for events, but rendering is
handled by the engine's OpenGL UI implementation.

## Verification Performed

- debug target compiled with `-Wall -Wextra` and no warnings
- standard debug executable linked successfully
- no legacy `SDL_CreateRenderer`, `SDL_RenderGeometry`, or SDLRenderer ImGui
  calls remain in engine source
- runtime initialized and remained responsive with OpenGL scene and ImGui active

## Intentional Follow-Up Work

- OpenGL debug callback
- shadow pass
- framebuffer abstraction
- sprite batching/instancing
- texture atlases
- 3D mesh queue
- modernized TTF/glyph rendering
