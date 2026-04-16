#pragma once

#include <span>

#include "sr/core/types.h"
#include "sr/core/color.h"
#include "sr/core/rect.h"
#include "sr/core/vec.h"
#include "sr/gfx/framebuffer.h"
#include "sr/gfx/texture.h"
#include "sr/gfx/render_state.h"

namespace sr {
    // Stateful façade over `raster::` free functions. Owns blend/sampler/wrap state and the scissor
    // rect; forwards both to the stateless raster layer on every call. The referenced FrameBuffer
    // must outlive this Renderer2D.
    class Renderer2D {
    public:
        explicit Renderer2D(FrameBuffer &fb) noexcept;

        auto clear(Color c = colors::black) noexcept -> void;

        auto clear(Pixel p) noexcept -> void;

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

        // sprites
        auto draw_sprite(const Texture &tex, Vec2i pos) noexcept -> void;

        auto draw_sprite_ex(const Texture &tex, Vec2f pos, Vec2f origin, Vec2f scale, f32 angle) noexcept -> void;

        // state
        auto set_blend_mode(BlendMode m) noexcept -> void { m_blend = m; }
        auto set_sampler_filter(SamplerFilter f) noexcept -> void { m_filter = f; }
        auto set_wrap_mode(WrapMode w) noexcept -> void { m_wrap = w; }

        [[nodiscard]] auto blend_mode() const noexcept -> BlendMode { return m_blend; }
        [[nodiscard]] auto sampler_filter() const noexcept -> SamplerFilter { return m_filter; }
        [[nodiscard]] auto wrap_mode() const noexcept -> WrapMode { return m_wrap; }

        // scissor — set_clip intersects with the framebuffer's full bounds
        auto set_clip(Recti r) noexcept -> void {
            m_clip = r.clipped(Recti::from_pos_size({0, 0}, {m_fb.width(), m_fb.height()}));
        }

        auto reset_clip() noexcept -> void {
            m_clip = Recti::from_pos_size({0, 0}, {m_fb.width(), m_fb.height()});
        }
        [[nodiscard]] auto clip() const noexcept -> Recti { return m_clip; }

        // escape hatch for low-level access (window present, custom passes, raw pixel reads)
        [[nodiscard]] auto framebuffer() noexcept -> FrameBuffer & { return m_fb; }
        [[nodiscard]] auto framebuffer() const noexcept -> const FrameBuffer & { return m_fb; }

    private:
        FrameBuffer &m_fb;
        Recti m_clip;
        BlendMode m_blend{BlendMode::Alpha};
        SamplerFilter m_filter{SamplerFilter::Nearest};
        WrapMode m_wrap{WrapMode::Clamp};
    };
}
