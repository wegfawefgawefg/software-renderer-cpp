# World / Gameplay Plan (Draft)

This renderer is growing into a simple game-sandbox. This doc describes a clean internal model.

## Scene vs Model
- `Model`: an asset. Can be multi-mesh / multi-material. No world behavior.
- `Scene`: a world container. Holds entities and systems state.

## World State (Fat Structs, Id-Style)
We are intentionally not building a “formal ECS” or a deep OOP hierarchy.

Practical, old-school approach:
- One `World` (or `Game`) struct owns *all* state.
- “Entities” can be plain structs with fields that get reused for different purposes.
- Systems are functions that mutate the `World` (or a subset of its fields).

Example:
- A `Door` can reuse the same fields as other things (health/timers/flags) because it is convenient.
- A lot of games ship like this. It is easy to debug and easy to iterate.

## Entities (Simple + Flexible)
We still want a way to place/render things in the world. The simplest version:

- `Entity {
    Transform, ModelRef, flags, ...
}`
- Optional fields can be present directly on the struct, or via “sub-structs”:
  - `PhysicsBody` (position/velocity)
  - `Collision` (shape type + params)
  - `Triggers` (warp zones, areas)

The key point: we don't need a framework. Keep it direct and pragmatic.

If we later want performance-oriented storage, we can migrate *specific* things to arrays, but it
should be driven by need, not architecture fashion.

## Pools (Hybrid: Entities + Specialized Arrays)
A very practical pattern (and what you used in `artificial`) is:

- A general `entities[]` container for things that are world-placed and share common behavior.
  - Player, NPCs, enemies, doors, triggers can all live here if it is convenient.
- Separate arrays/pools for "systems with their own lifecycle" where an `Entity` would be too fat:
  - `guns[]` (gun instances: reload state, heat, jam state, etc.)
  - `ground_guns[]` (gun pickups with position/size/sprite)
  - `inventory` (global and/or per-entity inventories)
  - items/pickups/projectiles/particles

Linking options:
- Store a simple integer handle/id (or `(id, version)` style handle) to reference pool entries.
- If the pool entry needs a transform, either:
  - store world position directly on the pool entry, or
  - store an `owner_entity_id` and read the transform from `entities[]`.

## Collision
- Static world: triangle mesh collider + spatial hash (current approach).
- Dynamic objects: simple shapes (sphere/AABB/capsule) + broadphase grid.

## Areas / Zones
An `Area` is a volume with “when player inside” effects:
- lighting overrides
- fog settings
- music/sfx triggers
- model visibility sets (load/unload chunks)

## Lighting Bake
Goal: allow “bake once per sun position” for static geometry (castle), but keep dynamic lighting for characters (Mario).

Proposed:
- For static model instances:
  - compute per-vertex lighting (or per-triangle constant) given a light rig
  - store baked values alongside mesh vertices for that instance
  - re-bake when sun moves (day/night), not every frame

## Interactions (Later)
- warp zones
- slopes + sliding
- pickups/throwables/breakables
- simple animations (keyframes) + billboards
