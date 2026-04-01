## Single clipping layer

Bounds checking happens in two places: FrameBuffer (`set_pixel`/`fill_hor_line`) and partially in raster:: (`blit`, `blit_ex`).
Redundant checks are nearly free but architecturally messy. Consider a clean single-layer approach without raw `data()` access.

## Migrate from minifb to SDL2/SDL3

minifb (C version 0.9.3) has no `mfb_set_title` — cannot change window title after creation.
The Rust minifb crate has `set_title`, the C library does not and has no plans to add it.

SDL solves this and provides:
- `SDL_SetWindowTitle` — FPS in title bar
- Full input support (gamepads, text input, clipboard)
- Window control (fullscreen, resize, multi-monitor)
- Audio if needed later

Rasterization stays ours — SDL only presents the pixel buffer
via `SDL_Texture` + `SDL_UpdateTexture`.
