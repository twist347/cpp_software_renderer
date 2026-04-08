#pragma once

#include <cassert>
#include <span>
#include <vector>

#include "sr/core/types.h"
#include "sr/core/color.h"

namespace sr {
    class FrameBuffer {
    public:
        static auto create(i32 width, i32 height) noexcept -> Result<FrameBuffer>;

        auto clear(Color c = colors::black) noexcept -> void;

        auto clear(Pixel argb) noexcept -> void;

        auto set_pixel(i32 x, i32 y, Pixel argb) noexcept -> void {
            if (in_bounds(x, y)) {
                m_buf[static_cast<std::size_t>(y) * m_width + x] = argb;
            }
        }

        auto set_pixel(i32 x, i32 y, Color c) noexcept -> void {
            set_pixel(x, y, c.to_argb());
        }

        [[nodiscard]] auto get_pixel(i32 x, i32 y) const noexcept -> Color {
            if (in_bounds(x, y)) {
                return Color::from_argb(m_buf[static_cast<std::size_t>(y) * m_width + x]);
            }
            return {};
        }

        auto fill_hor_line(i32 x0, i32 x1, i32 y, Pixel argb) noexcept -> void;
        auto fill_hor_line(i32 x0, i32 x1, i32 y, Color c) noexcept -> void;

        // x0 <= x1, all coords in bounds
        auto fill_hor_line_unchecked(i32 x0, i32 x1, i32 y, Pixel argb) noexcept -> void;
        auto fill_hor_line_unchecked(i32 x0, i32 x1, i32 y, Color c) noexcept -> void;

        auto set_pixel_unchecked(i32 x, i32 y, Pixel argb) noexcept -> void {
            assert(in_bounds(x, y));
            m_buf[static_cast<std::size_t>(y) * m_width + x] = argb;
        }

        auto set_pixel_unchecked(i32 x, i32 y, Color c) noexcept -> void {
            set_pixel_unchecked(x, y, c.to_argb());
        }

        [[nodiscard]] auto get_pixel_unchecked(i32 x, i32 y) const noexcept -> Color {
            assert(in_bounds(x, y));
            return Color::from_argb(m_buf[static_cast<std::size_t>(y) * m_width + x]);
        }

        [[nodiscard]] auto in_bounds(i32 x, i32 y) const noexcept -> bool {
            return x >= 0 && x < m_width && y >= 0 && y < m_height;
        }

        [[nodiscard]] auto data(this auto &self) noexcept { return self.m_buf.data(); }
        [[nodiscard]] auto span() const noexcept -> std::span<const Pixel> { return m_buf; }

        [[nodiscard]] auto width() const noexcept -> i32 { return m_width; }
        [[nodiscard]] auto height() const noexcept -> i32 { return m_height; }
        [[nodiscard]] auto stride() const noexcept -> i32 { return m_width; }

        [[nodiscard]] auto resize(i32 w, i32 h) noexcept -> Status;

    private:
        FrameBuffer(i32 w, i32 h, std::vector<Pixel> buf) noexcept;

        auto clip_hor_line(i32 &x0, i32 &x1, i32 y) noexcept -> Pixel *;

        i32 m_width;
        i32 m_height;
        std::vector<Pixel> m_buf;
    };
}
