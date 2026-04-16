#include "sr/gfx/renderer.h"
#include "sr/gfx/rasterizer.h"

#include <array>
#include <cmath>

namespace sr {
    namespace {
        // origin is a pivot point in local rect coordinates [0..size] that lands on pos;
        // consistent with blit_ex convention
        auto rotated_rect_vertices(Vec2f pos, Vec2f size, Vec2f origin, f32 angle) noexcept -> std::array<Vec2i, 4> {
            const f32 sin_a = std::sin(angle);
            const f32 cos_a = std::cos(angle);

            const auto transform = [&](Vec2f local) -> Vec2i {
                const f32 dx = local.x() - origin.x();
                const f32 dy = local.y() - origin.y();
                return {
                    static_cast<i32>(std::lround(pos.x() + dx * cos_a - dy * sin_a)),
                    static_cast<i32>(std::lround(pos.y() + dx * sin_a + dy * cos_a))
                };
            };

            return {
                transform({0.f, 0.f}),
                transform({size.x(), 0.f}),
                transform(size),
                transform({0.f, size.y()}),
            };
        }
    }

    Renderer2D::Renderer2D(FrameBuffer &fb) noexcept
        : m_fb{fb}, m_clip{Recti::from_pos_size({0, 0}, {fb.width(), fb.height()})} {
    }

    auto Renderer2D::clear(Color c) noexcept -> void {
        m_fb.clear(c);
    }

    auto Renderer2D::clear(Pixel p) noexcept -> void {
        m_fb.clear(p);
    }

    auto Renderer2D::draw_line(Vec2i a, Vec2i b, Color c) noexcept -> void {
        raster::draw_line(m_fb, m_clip, a, b, c, m_blend);
    }

    auto Renderer2D::draw_line_ex(Vec2i a, Vec2i b, f32 thickness, Color c) noexcept -> void {
        raster::draw_line_ex(m_fb, m_clip, a, b, thickness, c, m_blend);
    }

    auto Renderer2D::draw_rect(Vec2i a, Vec2i b, Color c) noexcept -> void {
        raster::draw_rect(m_fb, m_clip, a, b, c, m_blend);
    }

    auto Renderer2D::fill_rect(Vec2i a, Vec2i b, Color c) noexcept -> void {
        raster::fill_rect(m_fb, m_clip, a, b, c, m_blend);
    }

    auto Renderer2D::draw_circle(Vec2i center, i32 r, Color c) noexcept -> void {
        raster::draw_circle(m_fb, m_clip, center, r, c, m_blend);
    }

    auto Renderer2D::fill_circle(Vec2i center, i32 r, Color c) noexcept -> void {
        raster::fill_circle(m_fb, m_clip, center, r, c, m_blend);
    }

    auto Renderer2D::draw_ellipse(Vec2i center, i32 rx, i32 ry, Color c) noexcept -> void {
        raster::draw_ellipse(m_fb, m_clip, center, rx, ry, c, m_blend);
    }

    auto Renderer2D::fill_ellipse(Vec2i center, i32 rx, i32 ry, Color c) noexcept -> void {
        raster::fill_ellipse(m_fb, m_clip, center, rx, ry, c, m_blend);
    }

    auto Renderer2D::draw_triangle(Vec2i a, Vec2i b, Vec2i c, Color col) noexcept -> void {
        raster::draw_triangle(m_fb, m_clip, a, b, c, col, m_blend);
    }

    auto Renderer2D::fill_triangle(Vec2i a, Vec2i b, Vec2i c, Color col) noexcept -> void {
        raster::fill_triangle(m_fb, m_clip, a, b, c, col, m_blend);
    }

    auto Renderer2D::draw_polygon(std::span<const Vec2i> points, Color c) noexcept -> void {
        raster::draw_polygon(m_fb, m_clip, points, c, m_blend);
    }

    auto Renderer2D::fill_polygon(std::span<const Vec2i> points, Color c) noexcept -> void {
        raster::fill_polygon(m_fb, m_clip, points, c, m_blend);
    }

    auto Renderer2D::draw_rect_ex(Vec2f pos, Vec2f size, Vec2f origin, f32 angle, Color c) noexcept -> void {
        const auto vertices = rotated_rect_vertices(pos, size, origin, angle);
        raster::draw_polygon(m_fb, m_clip, vertices, c, m_blend);
    }

    auto Renderer2D::fill_rect_ex(Vec2f pos, Vec2f size, Vec2f origin, f32 angle, Color c) noexcept -> void {
        const auto vertices = rotated_rect_vertices(pos, size, origin, angle);
        raster::fill_polygon(m_fb, m_clip, vertices, c, m_blend);
    }

    auto Renderer2D::draw_sprite(const Texture &tex, Vec2i pos) noexcept -> void {
        raster::blit(m_fb, m_clip, tex, pos, m_blend);
    }

    auto Renderer2D::draw_sprite_ex(const Texture &tex, Vec2f pos, Vec2f origin, Vec2f scale, f32 angle) noexcept -> void {
        raster::blit_ex(m_fb, m_clip, tex, pos, origin, scale, angle, m_blend, m_filter, m_wrap);
    }
}
