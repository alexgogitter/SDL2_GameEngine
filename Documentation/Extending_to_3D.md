# Extending the Renderer to 3D

The current renderer is 2D-first, but its camera, shader, material and submission
boundaries were introduced so a 3D pass can be added without replacing the SDL
platform layer or object/component model.

## Already Available

- SDL-created OpenGL 3.3 core context
- GLAD function loading
- GLM matrices and vectors
- reusable `Shader` program wrapper
- abstract `Camera` interface
- functional perspective `Camera3D`
- command-submission pattern
- texture upload and caching
- linear/sRGB colour-space distinction

## Not Yet Implemented

- `Transform3D`
- mesh and vertex-format resources
- model import
- mesh renderer component
- depth-enabled 3D pass
- 3D materials
- directional, spot and 3D point lights
- skeletal animation
- environment maps
- 3D shadow maps
- physically based BRDF

`Camera3D` being present does not imply those systems exist.

## Recommended Architecture

Keep independent scene queues:

```text
2D components -> RenderQueue2D -> 2D pass
3D components -> RenderQueue3D -> depth/opaque/transparent 3D passes
UI callbacks  -> ImGui          -> final UI pass
```

The renderer may share texture objects and shader utilities, but 2D and 3D
materials should not be forced into one structure with dozens of irrelevant
fields.

## Suggested Implementation Order

1. Add `Transform3D` with position, quaternion rotation and scale.
2. Add `Mesh` resources with vertex/index buffers and an explicit vertex layout.
3. Add `Material3D` containing albedo, normal, roughness, metalness and emission.
4. Add `MeshRendererComponent` that submits mesh commands.
5. Add opaque depth-tested rendering using `Camera3D`.
6. Add directional light and a simple Blinn-Phong or Cook-Torrance shader.
7. Add glTF 2.0 import for Blender-authored assets.
8. Add transparent sorting and sky/environment rendering.
9. Add shadow maps after light and transform conventions are stable.

## Blender 3D Exchange

Use glTF 2.0 as the preferred eventual exchange format. It maps naturally to
Blender materials, meshes, scene nodes, skeletal animation and PBR texture
channels.

Avoid designing a custom binary model format before glTF import establishes the
actual runtime data requirements. A custom cooked format can be generated later
for faster loading.

## OpenGL Version

OpenGL 3.3 supports the initial 3D workflow, including vertex buffers,
framebuffers, instancing, uniform buffers and ordinary shadow maps.

Features such as shader storage buffers, compute shaders and modern bindless
resource strategies require newer versions. Raise the required context only when
a measured engine feature justifies reducing hardware compatibility.

## Transform Compatibility

Do not expand `Transform2D` into a half-2D, half-3D structure. Keep explicit
types and provide deliberate conversion helpers where needed:

```cpp
Transform3D makeBillboardTransform(const Transform2D& transform);
```

This avoids ambiguity around 2D half-size, 3D scale, rotation units and Z-layer
semantics.
