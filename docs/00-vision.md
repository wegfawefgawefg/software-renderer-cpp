#Vision

This repo is a from-scratch CPU software renderer in C++.

Goals:
- Clean separation between **assets** (loaded once) and **instances** (placed many times).
- A **scene** that is a world container (entities, cameras, lights), not an asset container.
- A renderer that is explicit about each pipeline stage: transform, cull/clip, raster, shade, compose.
- Small, readable files (target ~3-500 LOC each).
- Keep performance in mind, but prioritize correctness and good architecture first.

Non-goals (for now):
- Full PBR or modern GPU material systems.
- Complex scene graphs / animation systems.
- Massive dependency stacks.
