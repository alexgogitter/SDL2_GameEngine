# Cameras

The camera system separates scene projection from the renderer. Rendering code
accepts the abstract `Camera` interface, while concrete cameras choose how world
coordinates become clip-space coordinates.

## Shared Camera Interface

Every camera provides:

```cpp
virtual glm::mat4 getViewMatrix() const = 0;
virtual glm::mat4 getProjectionMatrix() const = 0;
virtual CameraProjection getProjectionType() const = 0;
virtual void setViewportSize(float width, float height) = 0;
```

The renderer uses:

```cpp
projection * view
```

through `Camera::getViewProjectionMatrix()`.

Only one camera is active in the current renderer:

```cpp
renderer.SetActiveCamera(&camera);
```

The renderer does not own the camera. The camera must remain alive while it is
active.

## Camera2D

`Camera2D` is an orthographic camera intended for the current Box2D world.

```cpp
Camera2D camera(windowWidth, windowHeight);
renderer.SetActiveCamera(&camera);
```

Its default position is the centre of its initial viewport. With a 1920 by 1080
viewport, world coordinates from `(0, 0)` to `(1920, 1080)` initially align with
the window.

### Coordinate Convention

The 2D world intentionally uses the existing SDL-style convention:

- origin near the top-left of the initial viewport
- positive X points right
- positive Y points down
- rotation is measured in radians
- Box2D positions are converted between pixels and metres

The orthographic projection reverses its top and bottom values to preserve this
positive-Y-down convention in OpenGL.

### Movement

```cpp
camera.setPosition({960.0f, 540.0f});
camera.setRotation(glm::radians(10.0f));
camera.setZoom(2.0f);
```

Zoom values are clamped to at least `0.01`. Larger values show a smaller world
area and therefore appear zoomed in.

### Screen to World

Mouse input arrives in window coordinates. Convert it before using it as a world
position:

```cpp
glm::vec2 mouseWorld = camera.screenToWorld({
    static_cast<float>(mouse.x),
    static_cast<float>(mouse.y)
});
```

This conversion accounts for camera position, rotation, zoom and viewport size.
The renderer updates the active camera's viewport every frame when the window is
resized.

### Camera Following

For a direct follow camera:

```cpp
camera.setPosition(player->transform.getPosition());
```

For smoother motion, interpolate toward the target using a frame-rate-independent
factor. Keep camera updates after physics has updated the target transform but
before rendering.

## Camera3D

`Camera3D` is a perspective camera provided as the first 3D extension point:

```cpp
Camera3D camera(width, height);
camera.setPosition({0.0f, 2.0f, 5.0f});
camera.lookAt({0.0f, 0.0f, 0.0f});
camera.setPerspective(60.0f, 0.1f, 1000.0f);
```

It currently provides valid view and perspective matrices, but the renderer does
not yet have a mesh queue, depth-enabled 3D pass, mesh resources, or 3D material
component. Creating a `Camera3D` does not by itself make `Render2D()` draw 3D
meshes.

## Future Multiple-Camera Work

Split-screen, minimaps, editor viewports and render-to-texture cameras will need
a scene/pass API rather than one global active camera. A likely shape is:

```cpp
renderer.BeginScene(camera, renderTarget);
renderer.Submit(...);
renderer.EndScene();
```

Do not place camera-specific behavior inside sprite components. Components
should continue submitting world-space data that any compatible camera can view.
