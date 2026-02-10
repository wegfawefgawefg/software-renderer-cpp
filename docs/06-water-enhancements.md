# Water Enhancements (Peach's Castle)

We are going to table water for a bit, but here is what we know and what we intend to do.

## What We Found

The Peach's Castle OBJ contains explicit "water geometry" as part of the same mesh.

- Model: `assets/models/peaches_castle.obj`
  - References: `mtllib peaches_castle.mtl`
  - Uses many materials (`usemtl Shape_###`)
- Material file: `assets/materials/peaches_castle.mtl`
  - The texture `6DAF90F6_c.png` appears to be water.
  - It is referenced by these materials:
    - `newmtl Shape_258` with `map_Kd 6DAF90F6_c.png`
    - `newmtl Shape_259` with `map_Kd 6DAF90F6_c.png`
- In the OBJ, those materials are used by geometry sections:
  - `usemtl Shape_258` around line ~6876
  - `usemtl Shape_259` around line ~6921

This means we can identify water triangles reliably by material name and/or by diffuse texture
path.

## Current Problem

Our physics collider is built from the entire castle triangle set, so the player collides with the
water surface like it is solid ground.

## Intentions

Short-term:
- Do not collide with water triangles.
- Keep water visible in the render, but exclude it from collision.

Longer-term:
- Add a simple "water controller":
  - When the player is in a water volume (or below water surface), switch movement parameters
    (gravity, drag, jump/float behavior).

Visual:
- Make the water feel alive:
  - Simple UV scrolling and/or a sinusoidal UV offset ("wiggle") in the water material.
  - Optionally alpha/blend if we want semi-transparent water.

## Options (Implementation Approaches)

### Option A: Physics Ignore by Material (Recommended)

During collider build, skip triangles whose material is known to be non-collidable (water).

Implementation detail:
- Build collision triangles using `Model.primitives[]` + `Primitive.material_index` so we can map
  faces to materials.
- Decide "collidable" per material.

Pros:
- Keeps the render model intact.
- Clean, flexible (later we can ignore fences, foliage, etc.).

Cons:
- Requires a small refactor of collision mesh build to be material-aware.

### Option B: Separate Collision Mesh

Create an alternate OBJ such as:
- `assets/models/peaches_castle_collision.obj`

that excludes water and other non-collidable parts.

Pros:
- Simple runtime.
- Common in real games.

Cons:
- Extra asset to maintain.
- Less flexible if we want to toggle collision per material dynamically.

### Option C: Water As Its Own OBJ

Extract the water faces into a separate model:
- `assets/models/peaches_castle_water.obj`

and render it separately.

Pros:
- Easy to apply special rendering (UV wiggle) and special physics rules.

Cons:
- Still need to ensure collision ignores it (either via separate collision mesh or filter).

## Identification Rules

Robust:
- Any material whose `map_Kd` equals `6DAF90F6_c.png` is water.

Pragmatic (current asset):
- Water materials are `Shape_258` and `Shape_259`.

