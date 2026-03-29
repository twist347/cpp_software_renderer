#pragma once

#include "sr/core/types.h"
#include "sr/core/color.h"
#include "sr/core/vec.h"
#include "sr/gfx/framebuffer.h"

namespace sr::raster {
    void put_pixel(FrameBuffer &fb, i32 x, i32 y, Color c) noexcept;

    void line(FrameBuffer &fb, Vec2i a, Vec2i b, Color c) noexcept;

    void draw_rect(FrameBuffer &fb, Vec2i a, Vec2i b, Color c) noexcept;
    void fill_rect(FrameBuffer &fb, Vec2i a, Vec2i b, Color c) noexcept;

    void draw_circle(FrameBuffer &fb, Vec2i center, i32 r, Color c) noexcept;
    void fill_circle(FrameBuffer &fb, Vec2i center, i32 r, Color c) noexcept;

    void fill_triangle(FrameBuffer &fb, Vec2i a, Vec2i b, Vec2i c, Color col) noexcept;
}
