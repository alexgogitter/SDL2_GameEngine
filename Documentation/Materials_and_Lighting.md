# 2D Materials and Lighting

The current renderer uses a forward-lit sprite shader. Each sprite command
contains resolved GPU textures and material parameters; each frame also contains
up to sixteen submitted point lights.

## Material2D

`Material2D` is the component-facing resource description:

| Field | Meaning | Default |
|---|---|---:|
| `albedo` | Base-colour and opacity texture | invalid |
| `alphaMask` | Grayscale alpha mask multiplied into albedo alpha | invalid |
| `diffuse` | Roughness/matte-response texture | invalid |
| `normal` | Tangent-space normal texture | invalid |
| `height` | Grayscale surface-height texture | invalid |
| `emission` | Self-illuminated colour texture | invalid |
| `specular` | Shiny/specular-response texture | invalid |
| `tint` | Multiplier applied to albedo | white |
| `normalStrength` | Multiplier for normal-map XY deviation | `1.0` |
| `heightScale` | Maximum pseudo-height in world units | `0.0` |
| `specularStrength` | Multiplier for specular-map brightness | `1.0` |
| `shininess` | Maximum specular highlight tightness | `32.0` |
| `lit` | Whether ambient and point lighting apply | `true` |
| `renderLayer` | Stable primary render-sort key | `0` |

Invalid texture handles resolve to OpenGL texture object `0`. The shader uses
boolean uniforms to avoid sampling missing maps.

Every material map also has a normalized UV source rectangle. The default
rectangle is the full texture. `AnimationControllerComponent` updates these
rectangles each frame when sampling from a TexturePacker atlas.

## Material2DRenderState

`Material2DRenderState` is an internal submission value containing resolved GPU
texture object names rather than resource handles. Gameplay code should normally
edit `Material2D`, not construct GPU state directly.

This separation is deliberate:

```text
asset path -> TextureHandle -> Resource_manager -> OpenGL texture
              Material2D                        Material2DRenderState
```

It leaves room for resource streaming and non-OpenGL backends without changing
gameplay components.

## Texture Slots

### Albedo

Albedo contains unlit surface colour and alpha. It is uploaded with
`GL_SRGB8_ALPHA8`, so sampling converts it into linear space before lighting.

Do not bake directional light or strong highlights into albedo if the asset will
be dynamically lit.

### Alpha mask

Alpha masks are linear grayscale textures. The shader multiplies the red channel
into albedo alpha before transparent pixels are discarded. This lets a sprite
sheet keep colour data on an opaque black background while a separate alpha row
defines the cutout silhouette.

### Diffuse / roughness

In this engine, the `diffuse` map is a linear roughness-style data map rather
than another colour texture:

```text
black: smoother surface, weaker matte response, stronger specular potential
white: rougher surface, stronger matte response, damped specular potential
```

The shader samples the red channel. Greyscale textures are recommended. Load
diffuse maps through `SpriteRendererComponent::setDiffuseMap()`, which uploads
them as linear data.

### Normal

Normals use tangent-space RGB values:

```text
R: local X, remapped from 0..1 to -1..1
G: local Y, remapped from 0..1 to -1..1
B: outward Z, remapped from 0..1 to -1..1
```

A flat surface is approximately `(128, 128, 255)` in an 8-bit texture.

Normal maps are uploaded as linear data. The shader rotates their XY direction
by the sprite's physics rotation so the apparent surface rotates with the object.

The engine currently expects an OpenGL-style positive-green convention. If
indentations appear raised vertically, invert the green channel during export or
asset processing.

### Height

Height uses the red channel as a value from zero to one. The shader multiplies it
by `heightScale` and uses it as the fragment's pseudo-Z coordinate.

Height mapping currently affects the light direction only. It does not perform
parallax mapping, relief mapping, displacement, self-occlusion, or shadow
casting.

### Emission

Emission is added after the lighting calculation. Black emits nothing. Coloured
emission textures are uploaded as sRGB and converted to linear space when
sampled.

### Specular / shiny

The `specular` map is a linear shiny-response map. Black texels produce no
highlight. Brighter texels create stronger and tighter Blinn-Phong-style
highlights from submitted point lights.

The shader uses the RGB value as highlight colour and derives gloss from its
luminance. `Material2D::specularStrength` scales the highlight brightness, while
`Material2D::shininess` controls the maximum tightness reached by white texels.

## Point-Light Model

`PointLight2D` contains:

```cpp
glm::vec3 position; // world X, world Y, height Z
glm::vec3 colour;   // linear RGB multiplier
float intensity;
float radius;       // planar world-space radius
bool castsShadows;  // reserved for the future shadow pass
```

For each fragment, the shader:

1. constructs a 3D direction from fragment `(x, y, height)` to light `(x, y, z)`
2. samples roughness from the diffuse map when present
3. computes diffuse response with `max(dot(normal, lightDirection), 0)`
4. computes quadratic planar attenuation inside the light radius
5. adds matte lighting to the albedo colour
6. adds specular highlights when a specular map is assigned
7. adds emission after dynamic lighting

The ambient light is currently hardcoded to `vec3(0.32)` in the renderer.

## Current Limits

- maximum sixteen submitted point lights per frame
- forward lighting repeats the light loop for every sprite fragment
- no spatial light culling other than the radius calculation in the shader
- no shadows
- no spotlights or directional lights
- no metalness or physically based BRDF
- no HDR render target, tone mapping or bloom
- transparent sprites use ordinary alpha blending and are sorted only by layer

These are implementation limits, not intended final design constraints.

## Render Ordering

Commands are stable-sorted by `renderLayer`, then submission order. A larger
layer draws later.

```cpp
sprite->getMaterial().renderLayer = 10;
```

Objects currently submit a colour quad before component sprites and an outline
afterward. Commands on the same layer keep that order.

## Shadow Roadmap

### First implementation: shadow volumes

1. Add `ShadowCaster2DComponent` with a convex polygon and height.
2. Optionally construct its initial polygon from `ColliderComponent`.
3. Cull casters against each light radius.
4. Extrude silhouette edges away from the point light.
5. Render the extrusion into an `R8` per-light mask or stencil buffer.
6. Multiply the light contribution by the sampled visibility.

This is the easiest design to debug visually and works naturally with Box2D
geometry.

### Later implementation: radial shadow maps

For many point lights, store the nearest occluder distance for each angle around
the light in a polar texture. Compare each fragment's radial distance with that
map and use neighbouring angular samples for soft edges.

### Deferred renderer

When the forward light loop becomes expensive, render sprites into a G-buffer:

- albedo and opacity
- world normal
- height and material properties
- emission

Then draw bounded light volumes into an HDR accumulation target and composite
the final scene. The current command and component interfaces were designed so
this change can remain inside the renderer.
