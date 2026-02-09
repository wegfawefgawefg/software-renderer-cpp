#Agent Instructions(software - renderer - cpp)

##Style Preferences - Prefer** simple C++**and** procedural organization** over heavy OOP.-
    It is fine(preferred)
to implement systems as free functions like `camera_update_orbit(...)`,
  `camera_update_third_person(...)`, etc., and choose between them with a small enum/int.
- Keep responsibilities separated **by file/module** more than by class hierarchy.
- Avoid mega files. Target **3-500 lines per file**. Split earlier rather than later.
- Keep code readable and debuggable. Correctness first, then performance.

## Formatting
- 4-space indentation, spaces only.
- Keep formatting enforced via `.clang-format` and `.editorconfig`.

## Architecture Conventions
- Assets (meshes/materials/textures) are loaded once and shared.
- World state (entities, transforms, physics state) is per-run/per-level.
- Rendering backend should be swappable later (software, OpenGL, etc.), so keep a clean seam:
  - “build render list” and “present” should not leak into simulation.

## “Do” and “Don’t”
- Do: small modules: `cli`, `input`, `sim`, `render`, `hud`, `present`, `settings`.
- Do: add flags/toggles when debugging tricky behavior.
- Don’t: add new dependencies unless they buy a lot (imgui is OK later if we commit to it).
- Don’t: bake game logic into renderer code.
