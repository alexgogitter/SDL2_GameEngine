# Colony Construction Prototype

The current executable is a proof-of-concept construction sandbox built on the
engine's OpenGL 2D renderer. It deliberately proves only the interaction layer
needed before colony simulation is added:

- moving around a world larger than the viewport;
- cursor-centred camera zoom;
- selecting, rotating, previewing, and placing structures on a grid;
- rejecting out-of-bounds and overlapping footprints;
- removing placed structures; and
- culling placed-object draw submissions outside the camera view.

## Controls

| Input | Action |
| --- | --- |
| `WASD` or arrow keys | Pan the camera |
| Move pointer to a screen edge | RTS-style camera pan |
| Middle mouse drag | Drag the camera |
| Mouse wheel | Zoom toward or away from the cursor |
| `1` to `4` | Select a construction item |
| `R` | Rotate the selected item by 90 degrees |
| Left mouse | Place the previewed item |
| Right mouse | Remove the item under the cursor |
| `Home` | Reset the camera |
| `Escape` | Exit |

## Current World Model

`ColonyPrototype` owns the gameplay state. The renderer only receives commands
for the visible terrain, objects, and placement preview. The sandbox currently
uses a fixed 128 by 128 grid of 128-pixel cells, giving a 16,384 by 16,384-pixel
world.

Placed structures store a type, minimum occupied cell, and quarter-turn
rotation. Collision is intentionally a simple rectangle scan because this
prototype is small. A production-scale world should replace that lookup with a
chunk-owned occupancy grid while preserving the same world/cell-facing API.

## Intended Next Slice

The next useful milestone is a colony simulation model that gives placed
structures stable identities and components for ownership, storage, production,
power, and construction state. After that, chunk serialization and a spatial
index can support very large maps without coupling simulation range to what the
camera happens to render.
