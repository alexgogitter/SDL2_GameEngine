# Animation Controller

`AnimationControllerComponent` is a component-local animation state machine for
`SpriteRendererComponent`. It advances frames over time and consumes string
events to transition between animation states.

It is not a global gameplay event bus. Gameplay code queues events directly on
the component that owns the animation.

## TexturePacker XML Workflow

The first supported import format is TexturePacker generic XML:

```xml
<TextureAtlas imagePath="Drone_Forward.png" width="896" height="3072">
    <sprite n="Image0001_45" x="384" y="2304" w="128" h="128"/>
</TextureAtlas>
```

Sprite names must follow:

```text
PrefixFrame_Angle
```

Examples:

- `Image0001_45`
- `Alpha0001_45`
- `Diffuse0001_45`
- `Normal0001_45`

Rows are treated as animation time through the frame number. Angle suffixes are
compiled into separate states. For a base clip named `Drone_Forward`, angles
`0`, `15` and `90` become:

```text
Drone_Forward_0
Drone_Forward_15
Drone_Forward_90
```

TexturePacker rotation and polygon packing are rejected in v1. Export rectangular
unrotated sprites.

## Prefix Mapping

| Prefix | Material slot | Colour space |
|---|---|---|
| `Image` | albedo | sRGB |
| `Alpha` | alpha mask | linear |
| `Diffuse` | diffuse/roughness | linear |
| `Normal` | normal | linear |
| `Height` or `Mist` | height | linear |
| `Emission` or `Emit` | emission | sRGB |
| `Specular` or `Gloss` | specular/shiny | linear |

The alpha mask is multiplied into albedo alpha before the shader discards
transparent pixels. This is useful for TexturePacker sheets with opaque black
backgrounds and separate white-on-black alpha rows.

## C++ Usage

```cpp
auto* sprite = new SpriteRendererComponent(object, resources);
object->add_Component(sprite);

auto* animator = new AnimationControllerComponent(
    object,
    resources,
    *sprite
);

animator->loadTexturePackerAtlas(
    "res/textures/drone/Drone_Forward.xml",
    "Drone_Forward",
    12.0f,
    true
);

animator->setDefaultState("Drone_Forward_0");
animator->addAnyTransition("Drone_Forward_45", "Drone_Forward_45");
animator->addAnyTransition("Drone_Forward_90", "Drone_Forward_90");

object->add_Component(animator);
```

Queue events from gameplay:

```cpp
animator->queueEvent("Drone_Forward_45");
```

The demo scene in `main.cpp` loads `res/textures/drone/Drone_Forward.xml` and
maps number keys to the generated angle states:

```text
1 -> Drone_Forward_0
2 -> Drone_Forward_15
3 -> Drone_Forward_30
4 -> Drone_Forward_45
5 -> Drone_Forward_60
6 -> Drone_Forward_75
7 -> Drone_Forward_90
```

Transition lookup order is:

1. current-state transition
2. any-state transition
3. direct state name match

If an event exactly matches a known clip name, the animator plays that clip from
any state.

## Frame Behavior

- `deltaTime` is interpreted as milliseconds.
- looping clips wrap after their final frame.
- non-looping clips stop on the final frame.
- switching state resets playback to frame zero.
- each frame updates the sprite material texture handles and per-map UV rects.

## Limitations

- one controller targets one `SpriteRendererComponent`
- no blend trees or cross-fades yet
- no external JSON controller file yet
- no rotated or polygon-packed TexturePacker sprites
- no per-frame duration metadata; every frame in a clip uses the clip FPS
