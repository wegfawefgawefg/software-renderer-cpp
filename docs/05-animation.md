# Animation Notes (Skeletal / Skinning)

## What We Implemented First

We start with the common "game engine 101" pipeline:

1. A **skeleton** is a hierarchy of joints (a tree). Each joint has a local TRS:
   - `T` translation (Vec3)
   - `R` rotation (Quat)
   - `S` scale (Vec3)

2. A **skinned mesh** stores, per vertex:
   - up to 4 joint indices
   - up to 4 weights (normalized to sum to 1)

3. The bind pose is captured by an **inverse bind matrix** per joint.

4. Each frame we:
   - sample the animation clip at time `t` to get joint local TRS
   - accumulate down the hierarchy to get joint global matrices
   - build joint skin matrices (global * inverse_bind)
   - apply Linear Blend Skinning (LBS) to every vertex:
     - `p' = sum_i (w_i * (M_i * p_bind))`

This is CPU skinning, which is simple and good enough at our "crunchy" resolutions.

## File Pointers

- Types:
  - `include/sr/assets/skeleton.hpp`
  - `include/sr/assets/animation.hpp`
  - `include/sr/assets/skinned_model.hpp`
- Math:
  - `include/sr/math/quat.hpp`
  - `include/sr/math/trs.hpp`
- FBX loading (model + baked clips):
  - `include/sr/assets/fbx_skinned_model_loader.hpp`
  - `src/sr/assets/fbx_skinned_model_loader.cpp`
- Skinning runtime:
  - `include/sr/anim/skinning.hpp`

## Implementation Notes / Gotchas

- We bake FBX animation to uniform samples (`sample_rate` Hz) on load. Runtime sampling is just
  lerp+slerp between neighboring frames.
- Joints are matched by **name** when loading animation clips from separate FBX files.
- We keep positions in "mesh geometry space" (model-local). We store a `world_to_model` matrix in
  `SkinnedModel` so we can keep the skinning math consistent with FBX's bind matrices.

