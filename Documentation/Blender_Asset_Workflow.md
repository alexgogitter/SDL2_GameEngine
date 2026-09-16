# Blender to Engine Asset Workflow

This is the target workflow for producing rendered 2D assets in Blender while
preserving data needed by the engine's real-time lighting.

The engine does not currently import `.blend` files. Blender is the authoring and
baking tool; runtime assets are image maps plus future metadata files.

## Recommended Asset Layout

Keep every material set in one folder:

```text
res/textures/crate/
  crate_albedo.png
  crate_diffuse.png
  crate_normal.png
  crate_height.png
  crate_emission.png
  crate_specular.png
  crate.material.json       # planned metadata format
```

For animated sprites:

```text
res/textures/robot/
  robot_albedo.png
  robot_diffuse.png
  robot_normal.png
  robot_height.png
  robot_emission.png
  robot_specular.png
  robot.animation.json      # planned metadata format
```

Every corresponding map must use the same canvas size, frame rectangles,
padding, pivot and orientation.

## Scene Setup in Blender

1. Model or assemble the source asset normally.
2. Use an orthographic camera aimed squarely at the intended sprite plane.
3. Lock camera resolution and orthographic scale before exporting any map.
4. Use a transparent film/background.
5. Put asset variants and animation frames into predictable collections or
   actions.
6. Keep a consistent unit-to-pixel policy across the project.

For top-down or isometric assets, document the camera angle and reuse it for the
entire asset family. Small camera differences create visible scale and normal
inconsistencies.

## Animation Render Settings

For a 64 by 64 final sprite, render a working sequence at 256 by 256 and reduce
it by exactly four using nearest-neighbour filtering. The larger working render
makes silhouettes more stable and gives room for pixel cleanup.

In Output Properties configure:

- resolution X: `256`
- resolution Y: `256`
- percentage: `100%`
- file format: PNG
- colour: RGBA
- colour depth: 8
- frame rate: normally 12 FPS for deliberately stepped pixel animation
- frame start/end: exclude a duplicated final loop pose
- frame step: use `2` when animating at 24 FPS but exporting on twos

In Render Properties configure:

- Film > Transparent: enabled
- Motion Blur: disabled
- Depth of Field: disabled on the camera

Use a fixed orthographic camera and keep the animated root/pivot stationary.
Blender appends frame numbers to animation output names.

The safest first workflow uses separate colour and data renders because albedo
and emission need display/sRGB encoding while normal, height, diffuse/roughness
and specular/shiny pixels must be written raw.

## Albedo Export

The albedo should represent intrinsic colour, not final scene illumination.

- Use an emission-based bake or another unlit colour pass.
- Keep alpha transparent outside the sprite.
- Avoid baked cast shadows, directional highlights and coloured scene lights.
- Gentle baked ambient occlusion may be stored separately when possible.
- Export losslessly as RGBA PNG.

The engine loads albedo as sRGB.

For Principled materials, enable the View Layer diffuse-colour pass. In the
Compositor:

1. enable Use Nodes
2. use the Render Layers `Diffuse Color`/`DiffCol` output
3. combine it with the Render Layers `Alpha` output using Set Alpha
4. connect it to a File Output node slot named `albedo_`
5. set the File Output node to PNG RGBA

Set Colour Management to Standard, Look None, Exposure 0, and Gamma 1 for this
colour render. Render Animation writes a numbered albedo sequence.

If a material does not contribute to Diffuse Color, add a material AOV carrying
its intended unlit base colour or temporarily render an emission-based copy of
that material.

## Normal Export

Bake tangent-space normals using the exact UVs and camera framing used by the
albedo.

- Treat the image as non-colour data in Blender.
- Export losslessly.
- Use `(128, 128, 255)` for flat or empty normal regions.
- Preserve albedo alpha or mask transparent pixels consistently.
- The engine expects OpenGL-style positive green.
- Do not apply sRGB colour correction to normal pixels.

For a rendered 3D model, create a dedicated material named `MAT_SpriteNormal`:

1. add Geometry and use its Normal output
2. add Vector Transform configured as Normal, World to Camera
3. separate XYZ
4. encode red as `X * 0.5 + 0.5`
5. encode green as `-Y * 0.5 + 0.5` because engine sprite Y points down
6. encode blue as `Z * 0.5 + 0.5`
7. combine RGB and connect it through an Emission shader to Material Output

Set the View Layer Material Override to `MAT_SpriteNormal`, set Colour Management
View Transform to Raw, and render the animation to a PNG RGBA File Output slot
named `normal_`. Restore the material override after rendering.

Validate the result on a sphere before exporting the complete animation:

- centre facing the camera should be approximately `(128, 128, 255)`
- the right side should be redder than the left
- the bottom should be greener than the top for the engine's +Y-down sprites

If those directions differ, correct the channel signs in the override material
rather than compensating differently for individual assets.

## Height Export

Height is a normalized grayscale representation of surface elevation:

- black means the lowest local surface
- white means the highest local surface
- values must be linear, not sRGB-corrected
- all frames should use a consistent height range

At runtime:

```cpp
sprite->getMaterial().heightScale = 12.0f;
```

The value is in the same pseudo-world units used for light height. Choose a
project convention rather than tuning every asset against arbitrary scales.

For a first height export, create `MAT_SpriteHeight`:

1. add Camera Data and use View Distance
2. use Map Range with the nearest expected surface distance as From Min
3. use the farthest expected surface distance as From Max
4. map the range to `1` for near and `0` for far, with Clamp enabled
5. connect the grayscale result through Emission to Material Output

Apply it as the View Layer Material Override, use the Raw view transform, and
render to a PNG RGBA File Output slot named `height_`.

The near/far mapping must remain fixed for every frame in one animation. Do not
normalize each frame independently or the apparent surface height will pulse.

## Emission Export

Use black for non-emissive pixels and colour for glowing regions. Emission is
added after lighting, so it remains visible in darkness.

Examples include:

- robot eyes
- neon signs
- lava cracks
- powered control panels

The engine loads coloured emission as sRGB.

Enable the View Layer emission pass and connect the Render Layers `Emission` or
`Emit` output to a PNG File Output slot named `emission_`. Render it in the same
Standard colour-managed pass as albedo. Use the object's alpha as the output
alpha when the pass does not already preserve it.

## Diffuse / Roughness Export

In this engine, the runtime `diffuse` map behaves like a roughness or
matte-response map, not a second colour texture.

- black means smoother and less matte
- white means rougher and more diffuse
- values must be linear, not sRGB-corrected
- greyscale is recommended; the shader currently samples the red channel

From a Blender Principled material, export the Roughness input or bake the
roughness value into an emission-style override material using the Raw view
transform. Write the result to a PNG RGBA File Output slot named `diffuse_`.

## Specular / Shiny Export

The runtime `specular` map controls shiny highlights from engine point lights.

- black means no specular highlight
- white means strong/tight highlight
- coloured values tint the highlight
- values must be linear, not sRGB-corrected

From a Blender material, export or bake the Specular/Specular IOR Level, Metallic
highlight mask, or a hand-authored shiny mask into an emission-style override
material using the Raw view transform. Write the result to a PNG RGBA File
Output slot named `specular_`.

For pixel art, specular maps often look best when simplified by hand after
downscaling. Keep crisp masks for metal trims, glass, wet areas, eyes and other
intentional highlight regions.

## Downscaling to 64 by 64

Do not use bilinear, bicubic, or Lanczos filtering. Reduce every corresponding
map with nearest-neighbour and keep names/frame numbers aligned.

In Aseprite, import the sequence, choose Sprite Size, set 64 by 64, and choose
Nearest Neighbor.

With ImageMagick:

```powershell
magick mogrify -path output64 -filter point -resize 64x64 input256\*.png
```

Palette reduction and hand cleanup are appropriate for albedo and emission.
Never palette-reduce normal, height, diffuse or specular data. Inspect alpha
edges after reduction; hard pixel art may benefit from an alpha threshold, but
apply the same silhouette mask to every map.

## Roughness and Other Maps

The current shader consumes runtime diffuse/roughness and specular/shiny maps,
but it is not a full physically based renderer. Metalness, occlusion, subsurface
and clearcoat maps should remain source data for now unless a future manifest
documents how to pack them.

## Sprite Sheets

All maps must use identical packing. If frame 7 occupies `(x, y, width, height)`
in the albedo atlas, frame 7 must occupy the same rectangle in every data map.

Recommended atlas rules:

- identical frame size across maps
- at least two to four pixels of edge extrusion around packed islands
- no rotation by the atlas packer
- power-of-two dimensions only if profiling shows a platform benefit
- a shared pivot stored as metadata rather than painted into the image

The runtime supports TexturePacker generic XML for rectangular, unrotated sprite
atlases. The animation controller expects names in this form:

```text
PrefixFrame_Angle
```

For example:

```text
Image0001_45
Alpha0001_45
Diffuse0001_45
Normal0001_45
```

Rows/frame numbers are animation time. Angle suffixes become separate animation
states such as `Drone_Forward_0`, `Drone_Forward_45` and `Drone_Forward_90`.
The current runtime prefix mapping is documented in
[Animation Controller](/Documentation/Animation_Controller.md).

## Naming Convention

Use suffixes consistently:

| Suffix | Colour space | Purpose |
|---|---|---|
| `_albedo` | sRGB | base colour and alpha |
| `_diffuse` | linear | runtime roughness/matte response |
| `_normal` | linear | tangent-space surface normal |
| `_height` | linear | normalized pseudo-height |
| `_emission` | sRGB | self-illuminated colour |
| `_specular` | linear | runtime shiny/specular response |
| `_ao` | linear | reserved ambient occlusion |

Consistent suffixes will allow a future Blender exporter and material-manifest
generator to discover maps automatically.

## Planned Material Manifest

The proposed format is intentionally not loaded yet:

```json
{
  "version": 1,
  "albedo": "crate_albedo.png",
  "diffuse": "crate_diffuse.png",
  "normal": "crate_normal.png",
  "height": "crate_height.png",
  "emission": "crate_emission.png",
  "specular": "crate_specular.png",
  "pivot": [0.5, 0.5],
  "pixelsPerMetre": 100,
  "normalStrength": 1.0,
  "heightScale": 12.0,
  "specularStrength": 1.0,
  "shininess": 32.0,
  "lit": true
}
```

Do not silently change the meaning of existing fields after this is implemented.
Add a version and migrate manifests deliberately.

## Validation Checklist

Before committing an asset:

- all maps open successfully
- all maps have identical dimensions
- all frames and pivots align
- albedo has no unwanted baked light
- normal map is treated as linear data
- normal green convention is correct
- height values use the project range
- diffuse/roughness values are linear and match the intended matte response
- transparent borders have adequate padding
- emission is black outside intended emissive areas
- specular values are linear and only highlight intentionally shiny areas
- the asset has been tested while rotating under a moving point light
