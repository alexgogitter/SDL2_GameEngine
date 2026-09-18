# SDL2 Game Engine Documentation

This is the canonical documentation index for the current engine. Public C++
headers also contain Doxygen-style comments for IntelliSense.

## Reading Path

Read the chapters in order for a new developer. Use the chapter pages as
reference once the corresponding subsystem is familiar.

## Chapter 1: Onboarding

- [Getting Started](/Documentation/Getting_Started.md) — dependencies, vcpkg,
  CMake presets, build commands, runtime setup, and first examples.
- [Troubleshooting](/Documentation/Troubleshooting.md) — build, startup,
  rendering, camera, input, resource, and shutdown diagnostics.

## Chapter 2: Architecture

- [Engine Architecture Overview](/Documentation/Overview.md) — subsystem map,
  ownership, frame flow, units, rendering, resources, components, and source
  layout.
- [SDL Renderer to OpenGL Migration](/Documentation/OpenGL_Migration.md) — the
  migration rationale, compatibility decisions, new dependencies, and follow-up
  work.

## Chapter 3: Scenes and Core Systems

- [Engine Foundations](/Documentation/Engine_Foundations.md) � C++ script hot reload,
  scene and prefab saving, asset browsing, logging, and standalone game packaging.

- [Objects and Components](/Documentation/Components.md) — object lifecycle,
  transforms, component ownership, physics, sprites, lights, and extension
  patterns.
- [Cameras](/Documentation/Cameras.md) — the shared camera interface,
  `Camera2D`, `Camera3D`, coordinates, zoom, and screen-to-world conversion.
- [Resource Manager](/Documentation/Resource_Manager.md) — texture and font
  caching, handles, colour spaces, bulk loading, and resource lifetime.

## Chapter 4: Rendering

- [OpenGL Renderer](/Documentation/OpenGL_Renderer.md) — context ownership,
  frame lifecycle, command queues, shaders, colour management, and ordering.
- [2D Materials and Lighting](/Documentation/Materials_and_Lighting.md) —
  material maps, texture slots, point lights, render state, limits, and shadows.
- [Current Renderer Status](/Documentation/Current_Status.md) — implemented
  features, incomplete foundations, planned milestones, and known constraints.

## Chapter 5: Assets and Animation

- [Blender to Engine Asset Workflow](/Documentation/Blender_Asset_Workflow.md) —
  Blender setup, texture map exports, sprite sheets, naming, and validation.
- [Animation Controller](/Documentation/Animation_Controller.md) — TexturePacker
  XML atlases, frame rectangles, angle states, transitions, and limitations.

## Chapter 6: 3D Direction

- [Extending the Renderer to 3D](/Documentation/Extending_to_3D.md) — available
  3D foundations, missing systems, recommended architecture, implementation
  order, and Blender/glTF direction.

## Chapter 7: Prototype and Maintenance

- [Colony Construction Prototype](/Documentation/Colony_Prototype.md) — current
  prototype controls, world model, and intended next slice.
- [Documentation Maintenance](/Documentation/Documentation_Maintenance.md) —
  update rules, canonical documents, IntelliSense comments, and review checklist.

## Accuracy Policy

`Current_Status.md` is the source of truth for whether a feature actually works.
Roadmap sections describe intended direction and are not promises that the code
already implements those features.
