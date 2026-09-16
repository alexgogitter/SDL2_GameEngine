# Resource Manager

`Resource_manager` caches OpenGL textures and SDL_ttf fonts behind stable integer
handles. It is a runtime cache, not yet an asynchronous asset database or
reference-counted content system.

## Lifetime Requirement

Construct the manager only after a valid OpenGL context exists and destroy it
before that context closes:

```cpp
Renderer renderer(...);
renderer.Renderer_Init();

{
    Resource_manager resources;
    // Load assets and run the scene.
}

renderer.Renderer_Close();
```

Texture loading and deletion call OpenGL directly and therefore require the
renderer's context to be current on the calling thread.

## Handles

`TextureHandle` is currently an unsigned integer owned by the resource layer.
It is not an OpenGL texture object.

```cpp
TextureHandle handle = resources.loadTexture("res/textures/crate.png");
unsigned int gpuTexture = resources.getTexture(handle);
```

Gameplay and material code should retain `TextureHandle`. Only renderer-facing
submission code should resolve it to `gpuTexture`.

`InvalidTextureHandle` is the maximum representable handle value and indicates
load failure or no assigned texture. OpenGL texture object zero indicates no
resolved GPU texture.

## Texture Loading

```cpp
TextureHandle albedo = resources.loadTexture(
    "res/textures/crate_albedo.png",
    TextureColourSpace::SRGB
);

TextureHandle normal = resources.loadTexture(
    "res/textures/crate_normal.png",
    TextureColourSpace::Linear
);
```

The load process is:

1. build a cache key from path and colour space
2. return the existing handle when already cached and resident
3. decode new images with SDL_image
4. convert decoded pixels to RGBA32
5. upload an OpenGL 2D texture
6. configure nearest filtering and clamp-to-edge wrapping
7. store path, dimensions, colour space and GPU object
8. return a stable engine handle

Texture paths are relative to the process working directory.

## Colour Space

Use the right colour space for the meaning of the pixels:

| Texture data | Colour space |
|---|---|
| albedo/base colour | sRGB |
| coloured emission | sRGB |
| alpha mask | linear |
| normal | linear |
| height | linear |
| diffuse/roughness | linear |
| specular/shiny | linear |
| occlusion | linear |
| masks/IDs | linear |

sRGB textures use `GL_SRGB8_ALPHA8`. Sampling converts them into linear values
before shader lighting. Linear textures use `GL_RGBA8` and retain numerical data
without gamma conversion.

The same path loaded once as sRGB and once as linear produces two cache entries.
That is intentional because their GPU formats and sampling meaning differ.

## Texture Metadata

`getTextureInfo()` returns a manager-owned, read-only pointer containing:

- original path
- OpenGL texture object
- width
- height
- colour space

The pointer remains subject to the resource manager's lifetime and container
mutations. Do not retain it as a long-lived asset reference.

## Release and Reload

`deleteTexture(handle)` deletes the OpenGL object but keeps cache metadata.
Calling `getTexture(handle)` afterward reloads the image from its original path
and stores a new OpenGL object behind the same engine handle.

This is the basis for future resource eviction, but the current implementation
does not automatically evict by memory pressure or usage.

## Bulk Loading

```cpp
resources.loadTextures({
    "res/textures/a.png",
    "res/textures/b.png"
});
```

`loadTextures()` assumes sRGB because it is intended as an albedo convenience.
Load data maps individually with an explicit linear colour space.

`loadLevelResources()` combines sRGB texture and font preloading.

## Fonts

Fonts are cached by path and point size:

```cpp
int fontHandle = resources.loadFont("res/fonts/comicz.ttf", 16);
TTF_Font* font = resources.getFont(fontHandle);
```

The returned `TTF_Font*` remains owned by the manager. Do not close it directly.

`deleteFont()` closes the font but preserves its path and size. A later
`getFont()` reopens it using the same handle.

## Destruction

The destructor:

- deletes every resident OpenGL texture
- closes every resident SDL_ttf font

It does not own or close the SDL/OpenGL subsystems themselves.

## Current Limitations

- synchronous loading on the calling thread
- no texture reference counting
- no automatic eviction budget
- no file watching or hot reload
- no texture arrays, cubemaps, compression, mipmaps or configurable samplers
- no central material-manifest loader
- no normalized/canonical path handling
- handles are not serialized asset IDs

Future resource work should preserve the distinction between stable engine
handles and backend-specific GPU objects.
