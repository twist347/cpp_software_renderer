# sr — C++23 Software Renderer

A from-scratch 2D software renderer. No GPU, no OpenGL — just pixels.

## Features

- **Primitives**: lines, rects, circles, ellipses, triangles, polygons (draw/fill)
- **Extended variants**: thick lines, rotated rects (`_ex` suffix)
- **Textures**: load images (stb_image), blit with alpha blending, scale/rotate
- **Windowing**: minifb backend, keyboard/mouse input, delta time, target FPS
- **Core**: `Vec<T,N>` template, `Color` (ARGB), `AABB2D`, `std::expected` error handling

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/examples/demo
```

Requires C++23 (GCC 14+).

## Status

2D MVP complete. Next: 3D (depth buffer, transforms, camera, OBJ loading).
