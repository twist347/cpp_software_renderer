#pragma once

#include <span>

#include "sr/core/types.h"
#include "sr/core/color.h"
#include "sr/core/vec.h"
#include "sr/gfx/framebuffer.h"

namespace sr {
    class Renderer2D {
    public:
        explicit Renderer2D(FrameBuffer &fb) noexcept;

        auto clear(Color c = colors::black) noexcept -> void;

        // primitives
        auto draw_line(Vec2i a, Vec2i b, Color c) noexcept -> void;
        auto draw_line_ex(Vec2i a, Vec2i b, f32 thickness, Color c) noexcept -> void;

        auto draw_rect(Vec2i a, Vec2i b, Color c) noexcept -> void;
        auto fill_rect(Vec2i a, Vec2i b, Color c) noexcept -> void;

        auto draw_circle(Vec2i center, i32 r, Color c) noexcept -> void;
        auto fill_circle(Vec2i center, i32 r, Color c) noexcept -> void;

        auto draw_ellipse(Vec2i center, i32 rx, i32 ry, Color c) noexcept -> void;
        auto fill_ellipse(Vec2i center, i32 rx, i32 ry, Color c) noexcept -> void;

        auto draw_triangle(Vec2i a, Vec2i b, Vec2i c, Color col) noexcept -> void;
        auto fill_triangle(Vec2i a, Vec2i b, Vec2i c, Color col) noexcept -> void;

        auto draw_polygon(std::span<const Vec2i> points, Color c) noexcept -> void;
        auto fill_polygon(std::span<const Vec2i> points, Color c) noexcept -> void;

        // extended
        auto draw_rect_ex(Vec2f pos, Vec2f size, Vec2f origin, f32 angle, Color c) noexcept -> void;
        auto fill_rect_ex(Vec2f pos, Vec2f size, Vec2f origin, f32 angle, Color c) noexcept -> void;

        [[nodiscard]] auto framebuffer() noexcept -> FrameBuffer & { return m_fb; }
        [[nodiscard]] auto framebuffer() const noexcept -> const FrameBuffer & { return m_fb; }

    private:
        FrameBuffer &m_fb;
    };
}
