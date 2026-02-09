# Architecture

## Data Model

### Assets (loaded once, shareable)
- `Texture`: immutable pixel data (RGBA8).
- `Material`: references textures + parameters (base color, flags like double-sided).
- `Mesh`: vertex/index buffers (single index stream; vertices include position + UV + optional normal).
- `Model`: a list of `Primitive`:
  - `Primitive { Mesh*, index_range, Material* }`

### Instances (placed in world)
- `Transform`: position/rotation/scale.
- `Entity`:
  - `Transform`
  - `Model*`
  - optional overrides (visibility, tint, layer)

### Scene (world container)
- `Scene`:
  - `std::vector<Entity>`
  - `Camera`
  - `Lights`

## Renderer Pipeline (CPU)
1. Build render list: expand `Scene.entities` into `RenderItem`s (primitive + world matrix).
2. Per-triangle:
   - transform to clip space (MVP)
   - frustum clip (clip-space planes)
   - perspective divide -> NDC -> screen
   - backface cull (configurable winding / double-sided)
3. Raster:
   - Z-buffer test per pixel
   - Texture sampling with repeat/clamp addressing
4. Present:
   - Copy framebuffer to SDL texture, present

## Performance Notes (planned)
- Per-object frustum culling via bounding sphere/AABB.
- Per-triangle culling/backface.
- Optional tile-based raster later for better cache and parallelism.

