# sr — C++23 Software Renderer

A from-scratch 2D software renderer. No GPU, no OpenGL — just pixels on a CPU-resident framebuffer, presented through [minifb](https://github.com/emoon/minifb).

## Features

### Primitives
- Lines (Bresenham + Cohen-Sutherland clipping), thick lines via polygon fill
- Rects, circles (midpoint), ellipses (midpoint, two-region Bresenham)
- Triangles and polygons (scanline fill with even-odd rule)
- Rotated rects (`_ex` suffix)

### Render state
- **Blend modes**: `None`, `Alpha` (Porter-Duff source-over, integer-only), `Additive`
- **Sampler filter**: `Nearest`, `Bilinear` (2×2 lerp)
- **Wrap mode**: `Clamp`, `Repeat`, `Mirror`
- State lives on `Renderer2D`; all dispatch is compile-time via templated internals

### Textures
- Load PNG/JPG via stb_image (RGBA → ARGB conversion at load time)
- `blit` (axis-aligned, pixel-exact) and `blit_ex` (scale + rotate + sampler)
- Texture-space origin convention (pivot lands on `pos`), consistent with raylib/SFML

### Core
- `Vec<T, N>` generic vector with deducing-this, structured bindings, broadcast ctor
- `Color` (ARGB8888) with integer-only blend, static `lerp`
- `AABB2D` with clipping and intersection
- `std::expected<T, Error>` for all fallible operations (`FrameBuffer::create`, `Texture::load`, `Window::create`)

### Platform
- `Window` over minifb: present, is_open, target FPS, FPS counter in title
- Input: keyboard (down/pressed/released edges), mouse buttons, position, scroll
- Delta time per frame

## Architecture

```
sr::core       — types, vec, color, aabb, error
sr::gfx
    raster::   — free functions, low-level primitives taking FrameBuffer&
    Renderer2D — stateful API over raster:: (holds blend/sampler state)
    FrameBuffer — owning pixel buffer (Pixel = u32 ARGB)
    Texture    — immutable image, loaded from file
sr::platform   — Window, Input
```

`raster::` is stateless; `Renderer2D` wraps it with render state. Public functions take runtime enum parameters; internally they dispatch to templated implementations (`write_pixel<M>`, `sample<F, W>`, `blit_ex_impl<M, F, W>`) so hot loops contain zero enum-checks.

FrameBuffer distinguishes checked public API from `*_unchecked` internals used after clipping; every `_unchecked` method asserts its precondition in debug.

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

./build/demos/01_primitives/demo_01_primitives
./build/demos/02_sprites/demo_02_sprites
```

Requires C++23 (GCC 14+ or Clang 18+). minifb is bundled under `thirdparty/`.

## Demos

- **`01_primitives`** — grid of all primitive types (filled/outlined, rotated).
- **`02_sprites`** — `blit` + `blit_ex` variants: scaled, rotated, scaled+rotated, mirrored, half-size.

## Status

2D MVP complete and hardened. Current renderer state + roadmap:

- [x] All primitives with alpha blending
- [x] Textures with nearest/bilinear sampling and clamp/repeat/mirror wrap
- [x] BlendMode (None / Alpha / Additive)
- [ ] Clip rect (scissor) on FrameBuffer
- [ ] Tile-parallel rasterizer
- [ ] 3D pipeline: Mat4, vertex/index buffers, MVP transform, z-buffer
- [ ] Programmable shaders (vertex + fragment as functors)
- [ ] Render targets / mip-maps
- [ ] SIMD rasterizer (edge functions, 2×2 quads)

See `TODO.md` for tracked issues.

## Not goals

Scene graph, material system with configs, GPU/RHI abstraction. The renderer stays stateless "give me draw calls, I rasterize" — no engine creep.
