#pragma once

#include <cassert>
#include <span>
#include <vector>

#include "sr/core/types.h"
#include "sr/core/color.h"

namespace sr {
    class Texture {
    public:
        [[nodiscard]] static auto load(const char *path) noexcept -> Result<Texture>;

        [[nodiscard]] static auto from_pixels(i32 w, i32 h, std::span<const Pixel> pixels) noexcept -> Result<Texture>;

        [[nodiscard]] auto width() const noexcept -> i32 { return m_width; }
        [[nodiscard]] auto height() const noexcept -> i32 { return m_height; }

        [[nodiscard]] auto get_pixel(i32 x, i32 y) const noexcept -> Color {
            if (in_bounds(x, y)) {
                return Color::from_argb(m_buf[static_cast<usize>(y) * m_width + x]);
            }
            return colors::transparent;
        }

        [[nodiscard]] auto get_pixel_argb(i32 x, i32 y) const noexcept -> Pixel {
            if (in_bounds(x, y)) {
                return m_buf[static_cast<usize>(y) * m_width + x];
            }
            return 0;
        }

        [[nodiscard]] auto get_pixel_unchecked(i32 x, i32 y) const noexcept -> Color {
            assert(in_bounds(x, y));
            return Color::from_argb(m_buf[static_cast<usize>(y) * m_width + x]);
        }

        [[nodiscard]] auto get_pixel_argb_unchecked(i32 x, i32 y) const noexcept -> Pixel {
            assert(in_bounds(x, y));
            return m_buf[static_cast<usize>(y) * m_width + x];
        }

        [[nodiscard]] auto in_bounds(i32 x, i32 y) const noexcept -> bool {
            return x >= 0 && x < m_width && y >= 0 && y < m_height;
        }

        [[nodiscard]] auto data() const noexcept -> const Pixel * { return m_buf.data(); }
        [[nodiscard]] auto stride() const noexcept -> i32 { return m_width; }

    private:
        Texture(i32 w, i32 h, std::vector<Pixel> buf) noexcept;

        i32 m_width;
        i32 m_height;
        std::vector<Pixel> m_buf;
    };
}
