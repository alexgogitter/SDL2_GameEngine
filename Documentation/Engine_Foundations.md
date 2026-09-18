# Engine Foundations

The Windows MSVC build now includes editor persistence, compiled C++ gameplay
modules, logging, and a standalone player.

## Working in the editor

- **File > Open Scene / Save Scene / Save Scene As** uses versioned `.scene` JSON.
  Shortcuts are Ctrl+O, Ctrl+S and Ctrl+Shift+S. Scene files contain object IDs,
  parent/child relationships, local transforms, colours, active/layer flags,
  registered components, enabled flags and serializable properties. Editor layer
  names and primary camera selections are also stored.
- The editor opens `game.json`'s `startupScene` on startup. The included scene is
  `res/scenes/main.scene`. Save changes before building a game.
- **File > Save Selected as Prefab** saves the selected object and its descendants.
  Double-click a `.prefab` in **Assets** to instantiate an independent copy with
  new IDs. Prefab instances are copies; linked overrides and propagation are not
  implemented.
- **Assets** browses `res/`, filters filenames, copies paths, opens scenes and
  instantiates prefabs. Drag a file onto a writable string property in Inspector
  to assign its path. Put distributable textures, scenes, atlases and prefabs
  under `res/` and use project-relative paths.
- **Preferences** always remains an undocked editor window. Its existing settings
  panels are still placeholders; this change addresses its docking behavior.
- **Play / Stop** snapshots and restores the entire scene, including objects or
  components created/deleted during play. Scene file operations are disabled
  during play to avoid saving simulation state accidentally.

Scene loading stages a complete replacement before replacing the live hierarchy.
Invalid JSON, unsupported versions, duplicate IDs, missing component types and
incompatible property types reject the load. Saves write a sibling temporary file
and replace the destination after the write succeeds. New/open operations warn
that the current scene will be replaced; save first to retain changes.

Sprite materials persist texture paths (not GPU handles), UVs, tint, lighting
parameters and draw rectangles. Animators persist their source atlases,
transitions and playback state. Mesh rendering currently supports the existing
built-in cube; this milestone does not add model import.

## C++ user scripts

In **Inspector > Components > Add Component**, use the **User Scripts** section
to create one `.cpp` file per script under `scripts/`. Give it a C++ identifier
such as `PlayerBehavior` or `InventoryScript`. The component viewer creates the
hook-only file and compiles it into `UserScripts.dll`.
Each script uses `REGISTER_USER_SCRIPT(YourClass)` at its end; that makes it
appear in Inspector's **Add Component > User Scripts** category. The included
`scripts/SpinScript.cpp` is a working example.
`scripts/PlayerBehaviorScript.cpp` and `scripts/InventoryScript.cpp` are empty
ready-to-edit examples.

Derive from `ScriptComponent` and override these hooks:

```cpp
void Setup() override;                       // Once, before the first PreUpdate
void PreUpdate(std::uint64_t milliseconds) override;
void Update(std::uint64_t milliseconds) override;
void Destroy() override;                     // Before removal or DLL unload
```

Save the source in your code editor, then return focus to Ricochet. The editor
detects the saved `.cpp` file, configures/builds the scripts, and reloads the
module. It does not rebuild continuously while you type. F7 remains available
when you want to rebuild without changing focus.

Each attached user-script component has an **Edit Script** button. By default it
runs VS Code's `code` command. To use another editor, set its executable path in
**Edit > Preferences > Editor > External editor executable**.

Setup runs after serialized properties are restored and only when the active,
enabled component first simulates. It runs again on reconstructed instances after
reload. Destroy can run on an instance that never reached Setup, so cleanup must
handle that case. Constructors should initialize local defaults only. Do not keep
background threads or callbacks into an unloaded module; release them in Destroy.
PreUpdate/Update use milliseconds, not seconds. C++ exceptions in these hooks are
logged and disable the component; native memory corruption/access violations are
not isolated from the editor.

Use `GetPropertyCount` / `GetProperty` with stable property keys to expose and
persist fields. Serializable writable properties survive reload. New properties
retain defaults, removed properties are ignored, and incompatible type changes
reject the reload. For additional structured state override `CaptureState` and
`RestoreState`; return a string (for example JSON). Runtime pointers must not be
serialized. Re-resolve object references from stable IDs after reconstruction.

Press **F7** or **File > Build User Scripts**. Configuration and compilation run
in a background process and their diagnostics stream into **Log**. Newly added
script files are picked up during configuration. A successful build reloads the
DLL. External builds are also detected at frame boundaries, at most twice per
second:

```powershell
cmake --build --preset msvc-debug --target UserScripts --parallel
```

The editor loads unique temporary copies, keeping the compiler's output DLL
unlocked. A candidate must have matching exports/API version, register correctly,
and reconstruct the current scene before replacing the old module. The old
instances are destroyed before their DLL is unloaded. Failed builds or rejected
reloads leave the old code active; see **Log** for the reason. **Reload Scripts**
forces another attempt. Reload reconstructs all objects, so raw object/component
pointers do not survive it. Only explicitly serialized state survives, not stacks,
statics, pending callbacks, or arbitrary private memory.

Keep engine and scripts on the same MSVC toolchain, architecture and build
configuration. Changes to engine headers/runtime ABI require closing the editor
and rebuilding both targets. Hot reload is for gameplay DLL changes. The player
loads the packaged module once and does not watch developer build outputs.

## Logging

`Logger::write(LogLevel::Info, "MyScript", "Setup completed");` writes to a
thread-safe session history and immediately flushes the log file. Warning and
Error levels are also available. SDL, resource-loading, shader and PhysX
messages use the same log. Build diagnostics include the compiler's file names,
line numbers and error codes.

**Log** provides text filtering, severity toggles, follow scrolling, copy, and
clear-view controls. Clearing the view does not erase the file. The last 4,000
entries are retained in memory; files append across launches.

- Editor: `logs/editor.log` in the project.
- Player: `player.log` in SDL's per-user `Ricochet/GamePlayer` preferences directory
  (normally under `%APPDATA%` on Windows).

## Building a game to share

Set `startupScene` in `game.json` to your saved scene under `res/`, then run:

```powershell
.\tools\build_msvc.bat Release StageGame
```

The complete folder is **`dist/Game/Release/`**:

```text
GamePlayer.exe
EngineRuntime.dll
imgui.dll
SDL, PhysX, image/font dependency DLLs and MSVC runtime DLLs
scripts/UserScripts.dll
game.json
res/
licenses/
```

`GamePlayer` also stages the files when its executable is rebuilt. Use `StageGame`
after asset-only changes so content is always refreshed. Debug builds use the
separate `dist/Game/Debug/` folder and are for development only. The engine and
editor still share EngineRuntime, so imgui.dll is required even though the player
has no editor interface.

To build and create the shareable ZIP in one command:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\package_game.ps1
```

Share **`dist/Game-windows-x64.zip`**. Recipients extract the whole archive and run
`GamePlayer.exe`; it resolves assets relative to itself, regardless of the shell's
working directory. No C++ compiler is required on the recipient's machine.
Windows x64 and an OpenGL 3.3-capable graphics driver are required. Test your final
game's content before sharing; assets outside `res/` are not automatically included.
The build helper currently uses this workspace's configured Visual Studio/vcpkg
locations; edit those paths on another development machine.

## Verification

```powershell
cmake --build --preset msvc-debug --target EngineFoundationTests
ctest --test-dir build/msvc-vs2022-x64 -C Debug --output-on-failure
.\build\msvc-vs2022-x64\Debug\SDL2_GameEngine.exe --smoke-test
.\dist\Game\Release\GamePlayer.exe --smoke-test
```

Foundation tests exercise hierarchy/property round trips, failed-load rollback,
atomic overwrites, prefab IDs, script lifecycle, DLL reload and rollback, component
dependencies, sprite/animation persistence, and log-file output. They require a
working SDL/OpenGL context and create their own temporary fixtures.
