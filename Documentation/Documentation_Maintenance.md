# Documentation Maintenance

Documentation is part of the implementation, not a cleanup task after it.

## Required Update Rule

Any change to one of the following must update documentation in the same unit of
work:

- public class or function signature
- ownership or lifetime rule
- frame order or render pass
- coordinate, scale or rotation convention
- material channel or colour-space convention
- asset filename or manifest convention
- shader input or light model
- build dependency or command
- implemented/planned feature status

## IntelliSense Comments

Public engine APIs use Doxygen-style comments so C++ IntelliSense displays useful
help at the call site.

Preferred form:

```cpp
/// Loads an image and uploads it to the active OpenGL context.
///
/// @param filePath Path relative to the process working directory.
/// @param colourSpace Sampling colour space for the texture contents.
/// @return A stable handle, or InvalidTextureHandle on failure.
TextureHandle loadTexture(
    const char* filePath,
    TextureColourSpace colourSpace
);
```

Comments should document information that the type system does not express:

- units such as pixels, metres, radians and degrees
- coordinate direction
- ownership transfer
- valid lifetime and required initialization order
- failure sentinel or error behavior
- whether a feature is currently active or reserved

Avoid comments that merely repeat a method name.

## Canonical Documents

| Subject | Canonical document |
|---|---|
| Setup and first run | `Getting_Started.md` |
| System relationships and frame flow | `Overview.md` |
| Renderer passes and APIs | `OpenGL_Renderer.md` |
| Cameras and coordinates | `Cameras.md` |
| Object/component ownership | `Components.md` |
| Materials, maps, lights and shadows | `Materials_and_Lighting.md` |
| Texture/font cache | `Resource_Manager.md` |
| Blender 2D export | `Blender_Asset_Workflow.md` |
| Future 3D architecture | `Extending_to_3D.md` |
| Feature truth table | `Current_Status.md` |
| Failure diagnosis | `Troubleshooting.md` |

Do not duplicate detailed rules into many documents. Link to the canonical
section and keep short summaries elsewhere.

## Change Checklist

Before declaring renderer work complete:

- update affected Doxygen comments
- update the canonical Markdown document
- update `Current_Status.md`
- add or revise a usage example
- ensure planned features are labelled as planned
- check Markdown links
- compile examples mentally against the current headers
- run `git diff --check`
- rebuild the debug target when source comments or declarations changed

## Date Policy

Only `Current_Status.md` needs a visible last-updated date. Other documents
should describe versioned behavior without becoming misleading merely because a
calendar date aged.
