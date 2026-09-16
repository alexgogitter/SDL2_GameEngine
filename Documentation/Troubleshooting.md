# Troubleshooting

## Build Problems

### `cannot open output file ... Permission denied`

The executable is probably still running. Close it in the window or stop the
process, then run `make debug` again.

### GLAD headers are missing

Confirm these files exist:

```text
external/glad/include/glad/glad.h
external/glad/include/KHR/khrplatform.h
external/glad/src/glad.c
```

The Makefile adds `external/glad/include` and compiles `glad.c`.

### OpenGL symbols fail to link

On Windows, confirm the Makefile includes `-lopengl32` after the source files.

### Box2D library is missing

Run:

```bash
make box2d
```

If its generated build is stale:

```bash
make clean-box2d
make box2d
```

## Startup Problems

### OpenGL context creation fails

The engine requests OpenGL 3.3 core. Update the GPU driver and verify the GPU
supports that version. Remote desktop and virtual machines may expose a reduced
OpenGL implementation.

### GLAD fails to load functions

GLAD must be initialized only after the SDL OpenGL context exists and has been
made current. Do not move `gladLoadGLLoader()` ahead of `SDL_GL_CreateContext()`.

### Shader files cannot be opened

Run from the repository root. Shader paths are currently relative:

```text
res/shaders/sprite2d.vert
res/shaders/sprite2d.frag
```

The packaged build must preserve the `res` folder beside the executable.

### SDL DLL is missing

Place `SDL2.dll`, `SDL2_image.dll` and `SDL2_ttf.dll` beside the executable or
add their directory to `PATH`.

## Rendering Problems

### Everything is black

Check:

- `Renderer::BeginFrame()` is called before submissions
- an active camera was assigned
- objects submit before `Renderer::Render2D()`
- the sprite shader compiled successfully
- at least ambient light remains enabled for lit materials
- material tint alpha is not zero

### UI appears but the world does not

ImGui has its own OpenGL pass. This symptom usually means the sprite shader,
camera, quad buffers or world submissions failed while the context itself is
working.

### World appears but ImGui does not

Confirm `Interface::create()` receives the SDL window and GL context, SDL events
are forwarded to `interface->update()`, and `interface->draw()` occurs after
`Renderer::Render2D()`.

### Texture is upside down

The current engine expects top-left-oriented image content and uses matching 2D
UVs. Do not independently flip only one map in a material set. If an asset tool
exports bottom-up data, make the conversion consistent across albedo, normal,
height, emission, diffuse and specular maps.

### Texture colours are too dark or washed out

Verify colour-space selection:

- albedo and coloured emission: sRGB
- normal, height, diffuse/roughness and specular/shiny: linear

The world pass enables an sRGB-capable framebuffer. Loading data maps as sRGB
corrupts their numerical values.

### Specular highlights never appear

Check:

- a specular map is assigned with `SpriteRendererComponent::setSpecularMap()`
- the specular map has non-black pixels where highlights should appear
- the specular map is loaded as linear data
- `material.specularStrength` is greater than zero
- the sprite is lit and a point light reaches it
- rough/diffuse pixels are not intentionally damping the highlight

### TexturePacker animation shows the wrong frame or black background

Check:

- TexturePacker rotation and polygon packing are disabled
- sprite names use `PrefixFrame_Angle`, for example `Image0001_45`
- every angle has matching `Image`, `Alpha`, `Diffuse` and `Normal` frame names
- the alpha row exists when the atlas PNG background is opaque black
- the controller state name includes the angle suffix, such as `Drone_Forward_45`

### Normal-map bumps point the wrong way

- Verify the normal map is loaded as linear.
- Verify flat areas are around `(128, 128, 255)`.
- Invert the green channel if the source uses the opposite Y convention.
- Confirm all maps share the same UV orientation.
- Test the sprite at zero rotation, then while rotating.

### Rotating sprite has stationary lighting detail

The fragment shader must rotate tangent-space normal XY by the sprite rotation.
Check `uSpriteRotation` and the normal rotation in `sprite2d.frag`.

### Light does not affect a sprite

Check:

- `material.lit` is `true`
- the light component is attached to an object that is drawn
- the light radius reaches the sprite
- intensity is greater than zero
- light height is sensible relative to material height
- fewer than sixteen lights were submitted earlier in the frame

### `castsShadows` has no visible effect

This is expected. The property is reserved, but the shadow pass has not been
implemented yet.

## Camera and Input Problems

### Mouse spawning is offset after moving the camera

Use `Camera2D::screenToWorld()` rather than using SDL mouse coordinates directly.

### World changes scale after a resize

The renderer updates the active camera's viewport each frame. If a separate
camera cache stores projection matrices, invalidate it when `setViewportSize()`
is called.

### Box2D object and sprite separate

Do not edit only `Object::transform` on an active physics body. Use the rigid
body's teleport or velocity APIs so Box2D remains authoritative.

## Resource Problems

### Texture handle is invalid

`loadTexture()` returns `InvalidTextureHandle` on failure. Check file paths and
SDL_image error output before storing the handle.

### Crash while shutting down

Destroy `Resource_manager` before closing the OpenGL context. Its destructor
deletes GPU textures.

### Texture reload behaves unexpectedly

The cache key includes path and colour space. The same file loaded as sRGB and
linear intentionally creates two resources.

## Diagnostic Improvements Still Needed

- OpenGL debug callback in debug builds
- shader uniform validation
- visible missing-texture fallback
- asset manifest validation
- framebuffer completeness checks for future render targets
- automated render smoke tests
