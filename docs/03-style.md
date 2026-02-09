#Code Style

## Guiding Principles
- Prefer **procedural modules** over heavy OOP.
- Prefer **separation by file** (concerns/modules) over deep class trees.
- Keep files small (target **3-500 LOC**).

## Cameras
We intentionally support multiple camera control schemes using functions and a small mode switch.

Example pattern:
- `camera_update_orbit(...)`
- `camera_update_fly(...)`
- `camera_update_third_person(...)`

And a simple selector (enum/int) in the app state.

## Debug UI
- Start with a tiny built-in HUD (FPS, toggles).
- Later we can add imgui for advanced debug panels (materials, perf counters, render modes).
