# software-renderer-cpp

CPU software renderer in C++ (SDL2).

## Build

```bash
cmake -S . -B build_cpp -DCMAKE_BUILD_TYPE=Release
cmake --build build_cpp -j
./build_cpp/renderer
```

## Docs

See `docs/` for the current plan and architecture.

## Legacy Reference

The original C (pikuma-based) code was kept for reference under `legacy/pikuma/src/`.
