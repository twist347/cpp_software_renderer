#include "sr/gfx/renderer.h"
#include "sr/gfx/rasterizer.h"

#include <array>
#include <cmath>

namespace sr {
    static auto rotate_point(Vec2f p, Vec2f origin, f32 sin_a, f32 cos_a) noexcept -> Vec2i;

    Renderer2D::Renderer2D(FrameBuffer &fb) noexcept
        : m_fb{fb} {
    }

    auto Renderer2D::clear(Color c) noexcept -> void {
        m_fb.clear(c);
    }

    auto Renderer2D::draw_line(Vec2i a, Vec2i b, Color c) noexcept -> void {
        raster::draw_line(m_fb, a, b, c);
    }

    auto Renderer2D::draw_line_ex(Vec2i a, Vec2i b, f32 thickness, Color c) noexcept -> void {
        raster::draw_line_ex(m_fb, a, b, thickness, c);
    }

    auto Renderer2D::draw_rect(Vec2i a, Vec2i b, Color c) noexcept -> void {
        raster::draw_rect(m_fb, a, b, c);
    }

    auto Renderer2D::fill_rect(Vec2i a, Vec2i b, Color c) noexcept -> void {
        raster::fill_rect(m_fb, a, b, c);
    }

    auto Renderer2D::draw_circle(Vec2i center, i32 r, Color c) noexcept -> void {
        raster::draw_circle(m_fb, center, r, c);
    }

    auto Renderer2D::fill_circle(Vec2i center, i32 r, Color c) noexcept -> void {
        raster::fill_circle(m_fb, center, r, c);
    }

    auto Renderer2D::draw_ellipse(Vec2i center, i32 rx, i32 ry, Color c) noexcept -> void {
        raster::draw_ellipse(m_fb, center, rx, ry, c);
    }

    auto Renderer2D::fill_ellipse(Vec2i center, i32 rx, i32 ry, Color c) noexcept -> void {
        raster::fill_ellipse(m_fb, center, rx, ry, c);
    }

    auto Renderer2D::draw_triangle(Vec2i a, Vec2i b, Vec2i c, Color col) noexcept -> void {
        raster::draw_triangle(m_fb, a, b, c, col);
    }

    auto Renderer2D::fill_triangle(Vec2i a, Vec2i b, Vec2i c, Color col) noexcept -> void {
        raster::fill_triangle(m_fb, a, b, c, col);
    }

    auto Renderer2D::draw_polygon(std::span<const Vec2i> points, Color c) noexcept -> void {
        raster::draw_polygon(m_fb, points, c);
    }

    auto Renderer2D::fill_polygon(std::span<const Vec2i> points, Color c) noexcept -> void {
        raster::fill_polygon(m_fb, points, c);
    }

    auto Renderer2D::draw_rect_ex(Vec2f pos, Vec2f size, Vec2f origin, f32 angle, Color c) noexcept -> void {
        const f32 sin_a = std::sin(angle);
        const f32 cos_a = std::cos(angle);
        const Vec2f o = pos + origin;

        const std::array<Vec2i, 4> vertices = {
            rotate_point(pos, o, sin_a, cos_a),
            rotate_point({pos.x() + size.x(), pos.y()}, o, sin_a, cos_a),
            rotate_point(pos + size, o, sin_a, cos_a),
            rotate_point({pos.x(), pos.y() + size.y()}, o, sin_a, cos_a),
        };

        raster::draw_polygon(m_fb, vertices, c);
    }

    auto Renderer2D::fill_rect_ex(Vec2f pos, Vec2f size, Vec2f origin, f32 angle, Color c) noexcept -> void {
        const f32 sin_a = std::sin(angle);
        const f32 cos_a = std::cos(angle);
        const Vec2f o = pos + origin;

        const std::array<Vec2i, 4> vertices = {
            rotate_point(pos, o, sin_a, cos_a),
            rotate_point({pos.x() + size.x(), pos.y()}, o, sin_a, cos_a),
            rotate_point(pos + size, o, sin_a, cos_a),
            rotate_point({pos.x(), pos.y() + size.y()}, o, sin_a, cos_a),
        };

        raster::fill_polygon(m_fb, vertices, c);
    }

    // inner funcs

    static auto rotate_point(Vec2f p, Vec2f origin, f32 sin_a, f32 cos_a) noexcept -> Vec2i {
        const f32 dx = p.x() - origin.x();
        const f32 dy = p.y() - origin.y();
        return {
            static_cast<i32>(origin.x() + dx * cos_a - dy * sin_a),
            static_cast<i32>(origin.y() + dx * sin_a + dy * cos_a)
        };
    }
}
