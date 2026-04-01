#pragma once

#include <span>

#include "sr/core/types.h"
#include "sr/core/color.h"
#include "sr/core/vec.h"
#include "sr/gfx/framebuffer.h"
#include "sr/gfx/texture.h"

namespace sr::raster {
    auto draw_line(FrameBuffer &fb, Vec2i a, Vec2i b, Color c) noexcept -> void;
    auto draw_line_ex(FrameBuffer &fb, Vec2i a, Vec2i b, f32 thickness, Color c) noexcept -> void;

    auto draw_rect(FrameBuffer &fb, Vec2i a, Vec2i b, Color c) noexcept -> void;
    auto fill_rect(FrameBuffer &fb, Vec2i a, Vec2i b, Color c) noexcept -> void;

    auto draw_circle(FrameBuffer &fb, Vec2i center, i32 r, Color c) noexcept -> void;
    auto fill_circle(FrameBuffer &fb, Vec2i center, i32 r, Color c) noexcept -> void;

    auto draw_ellipse(FrameBuffer &fb, Vec2i center, i32 rx, i32 ry, Color c) noexcept -> void;
    auto fill_ellipse(FrameBuffer &fb, Vec2i center, i32 rx, i32 ry, Color c) noexcept -> void;

    auto draw_triangle(FrameBuffer &fb, Vec2i a, Vec2i b, Vec2i c, Color col) noexcept -> void;
    auto fill_triangle(FrameBuffer &fb, Vec2i a, Vec2i b, Vec2i c, Color col) noexcept -> void;

    auto draw_polygon(FrameBuffer &fb, std::span<const Vec2i> points, Color c) noexcept -> void;
    auto fill_polygon(FrameBuffer &fb, std::span<const Vec2i> points, Color c) noexcept -> void;

    // bit block transfer (BitBLT)
    auto blit(FrameBuffer &fb, const Texture &tex, Vec2i pos) noexcept -> void;
    auto blit_ex(FrameBuffer &fb, const Texture &tex, Vec2f pos, Vec2f origin, Vec2f scale, f32 angle) noexcept -> void;
}
