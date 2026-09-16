# Objects and Components

The engine uses `Object` as an owning entity container and `Component` as the
base class for attachable behavior.

## Object Lifecycle

An object owns:

- its `Transform2D`
- its attached components
- its child objects
- a fallback draw colour

Each frame:

```cpp
object->update(deltaTime);
object->draw(&renderer);
```

`update()` performs the following work:

1. calls the object's virtual `Update()` method
2. updates every component
3. updates every child

`draw()` submits:

1. the object's fallback colour quad only when no sprite renderer is attached
2. draw commands from every component
3. a collider outline in debug builds when a collider component exists
4. all child objects

The fallback quad no longer sits behind attached sprites. Sprite visuals are
owned by `SpriteRendererComponent`; collider debug visualization is a separate
debug-only outline.

## Transform2D Convention

`Transform2D` contains:

```cpp
glm::vec2 position;
glm::vec2 scale;
float rotation;
```

Important: in renderer submissions, `scale` is treated as the submitted quad's
half-size.

```cpp
transform.setScale(pixelSize * 0.5f);
```

A scale of `(32, 32)` produces a 64 by 64 quad. This convention matches the
legacy fallback drawing path and the renderer's quad command format. It no
longer defines the size of a `SpriteRendererComponent`; sprites own a separate
local draw rectangle.

Rotation uses radians. `RigidbodyComponent` copies the Box2D body's rotation
into the shared transform, which also rotates the sprite draw rect and the
sprite's tangent-space normal.

## Component API

A component implements update behavior and optionally drawing behavior:

```cpp
class ExampleComponent final : public Component
{
public:
    ExampleComponent(Object* parent)
        : Component("Example", parent)
    {
    }

private:
    void Update(std::uint64_t deltaTime) override
    {
    }

    void Draw(Renderer* renderer) override
    {
    }
};
```

`Draw()` should submit renderer commands. It should not call raw OpenGL directly.
That restriction keeps render ordering, future batching and multi-pass lighting
under renderer control.

## Ownership

Components are currently added as raw pointers:

```cpp
object->add_Component(new ExampleComponent(object));
```

Ownership transfers to `Object`. Do not delete the component yourself after a
successful call.

Components are keyed by their `name`. Adding another component with the same name
deletes and replaces the existing component. Removing a component also deletes
it.

Child-object ownership follows the same transfer model.

## Physics Components

`PhysicsBoxObject` creates:

- `RigidbodyComponent` named `Rigidbody2D`
- `ColliderComponent` named `BoxCollider2D`

The rigid body owns the Box2D body. Box2D destroys attached shapes when the body
is destroyed.

The update direction is currently physics to graphics:

```text
Box2D body -> RigidbodyComponent -> Object Transform2D -> renderer
```

Use `RigidbodyComponent::teleportPixels()` rather than editing only the transform
when deliberately moving an existing physics body.

## SpriteRendererComponent

`SpriteRendererComponent` owns a `Material2D` value and a pointer to the resource
manager that resolves its texture handles. It also owns a local draw rectangle
used only for visual rendering.

```cpp
auto* sprite = new SpriteRendererComponent(
    object,
    resources,
    "res/textures/crate_albedo.png"
);

sprite->setNormalMap("res/textures/crate_normal.png");
sprite->setHeightMap("res/textures/crate_height.png");
sprite->setEmissionMap("res/textures/crate_emission.png");
sprite->setDiffuseMap("res/textures/crate_diffuse.png");
sprite->setSpecularMap("res/textures/crate_specular.png");
sprite->setDrawSize({64.0f, 64.0f});
sprite->getMaterial().specularStrength = 0.8f;
sprite->getMaterial().shininess = 48.0f;

object->add_Component(sprite);
```

In this engine, `setDiffuseMap()` means a linear roughness/matte-response map,
not a second colour map. `setSpecularMap()` means a linear shiny/specular-response
map.

The draw rect is local to the parent object:

```cpp
sprite->setDrawRect(-32.0f, -48.0f, 64.0f, 96.0f);
```

`x` and `y` are the local top-left corner relative to the parent centre.
`width` and `height` are the full rendered size. The parent transform supplies
world position and rotation; the collider size remains independent.

When an albedo texture is loaded and no explicit draw rect was set, the sprite
auto-centres a draw rect using the albedo texture's pixel dimensions.

`TextureComponent` is currently an alias for `SpriteRendererComponent`, so older
code remains source-compatible.

## AnimationControllerComponent

`AnimationControllerComponent` targets one `SpriteRendererComponent` and updates
its material handles, per-map UV rects and draw size every frame.

```cpp
auto* animator = new AnimationControllerComponent(object, resources, *sprite);
animator->loadTexturePackerAtlas(
    "res/textures/drone/Drone_Forward.xml",
    "Drone_Forward",
    12.0f,
    true
);
animator->setDefaultState("Drone_Forward_0");
animator->queueEvent("Drone_Forward_45");
object->add_Component(animator);
```

Events are local to the component. See
[Animation Controller](/Documentation/Animation_Controller.md) for TexturePacker
naming rules and transition behavior.

## Light2DComponent

A light follows the transform of its parent object:

```cpp
object->add_Component(new Light2DComponent(
    object,
    {1.0f, 0.8f, 0.6f},
    1.5f,
    400.0f,
    120.0f
));
```

Constructor parameters are colour, intensity, planar radius and height above the
2D surface. Height is the Z coordinate used by the normal-mapped lighting
calculation.

`castsShadows` is stored but not consumed by a shadow pass yet.

## Recommended Improvements

The current ownership model works, but future work should consider:

- `std::unique_ptr<Component>` parameters
- typed component lookup such as `getComponent<T>()`
- separating logical scale from sprite/collider size
- explicit render layers on objects and components
- scene-owned entity IDs for serialization
