# Current Renderer Status

Last updated: 17 September 2026.

This file distinguishes working features from foundations and planned work. Keep
it current whenever a rendering milestone changes state.

## Editor and Runtime Foundations

Implemented: compiled C++ gameplay DLL reload with Setup/PreUpdate/Update/Destroy,
versioned scene files, prefab copies, an asset browser, a file-backed Log window,
undockable Preferences, complete Play/Stop scene restoration, and GamePlayer
packaging under `dist/Game/<configuration>/`. See
[Engine Foundations](/Documentation/Engine_Foundations.md) for contracts and limits.

## Implemented

- SDL2 window and event platform
- OpenGL 3.3 core context
- GLAD 0.1.36 function loading
- GLM matrix math
- sRGB-capable world framebuffer
- command-queued 2D quad and outline rendering
- stable render-layer ordering
- per-map atlas UV/source rectangles
- albedo texture sampling
- alpha-mask cutout sampling
- diffuse/roughness matte-response mapping
- tangent-space normal mapping
- pseudo-height lighting input
- emission mapping
- specular/shiny point-light highlights
- material tinting and unlit materials
- up to sixteen forward-rendered point lights
- orthographic `Camera2D`
- screen-to-world mouse conversion
- perspective `Camera3D` matrices
- OpenGL ImGui renderer
- OpenGL texture caching and colour-space selection
- Box2D transform synchronization
- `SpriteRendererComponent`
- independent sprite draw rectangles
- `AnimationControllerComponent`
- TexturePacker generic XML atlas animation
- debug-build collider outlines using collider dimensions
- `Light2DComponent`
- compatibility `TextureComponent` alias

## Foundations Present, Feature Incomplete

- 3D mesh pass renders the built-in cube; external model import is still pending
- `PointLight2D::castsShadows`: property exists; no shadow pass consumes it
- height maps: influence light direction; no parallax or displacement
- render layers: stable integer ordering; no transparent depth policy
- animation controller: no blends, JSON controller file or rotated atlas support
- TTF rendering: transient textured quad path exists; no glyph atlas renderer

## Planned Next Rendering Milestones

1. Normal-mapped Blender test asset with validated channel conventions.
2. `ShadowCaster2DComponent` and collider-derived shadow volumes.
3. Light/caster spatial culling.
4. Off-screen framebuffer and compositing pass.
5. Deferred 2D G-buffer when forward-light profiling justifies it.
6. Versioned material manifests and Blender export automation.
7. OpenGL debug callback and render diagnostics.
8. `Transform3D`, mesh resources and `MeshRendererComponent`.
9. glTF-based Blender 3D import.

## Known Constraints

- Supported Windows build uses MSVC, CMake and vcpkg; Makefile delegates to it.
- Editor uses the project directory; GamePlayer resolves packaged assets relative to its executable.
- `Transform2D::scale` still means half-size for submitted renderer quads.
- OpenGL objects are represented internally as unsigned integers.
- Components use raw-pointer ownership transfer.
- Each render pass uses one camera; editor camera outputs select separate scene cameras.
- Scene/Game View render targets exist; post-processing is not implemented.
- No shadows, metalness, physically based BRDF or HDR exist.
- ImGui uses a fixed uploaded font atlas rather than dynamic texture updates.
