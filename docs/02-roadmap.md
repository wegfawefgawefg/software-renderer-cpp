#Roadmap

## Milestone 0: Repo Scaffold (now)
- CMake builds C++ app (`renderer`).
- SDL window + CPU framebuffer display.
- `docs/` exists and captures architecture.

## Milestone 1: Math + Framebuffer
- Vec/Mat types
- Camera + projection helpers
- Z-buffer

## Milestone 2: Raster Core
- Triangle raster (filled + textured)
- UV repeat addressing
- Backface culling + clip-space frustum clipping

## Milestone 3: Assets & Scene
- `Texture`, `Material`, `Mesh`, `Model`, `Entity`, `Scene`
- Render list build

## Milestone 4: Loading
- OBJ + MTL loading into `Model` (multi-material).
- Texture loading and caching.

## Milestone 5: Demo Scene
- Load `assets/models/peaches_castle.obj`
- Load Mario model (format TBD later)
