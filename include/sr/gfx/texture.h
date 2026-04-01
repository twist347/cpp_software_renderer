#pragma once

#include <vector>

#include "sr/core/types.h"
#include "sr/core/color.h"

namespace sr {
    class Texture {
    public:
        static auto load(const char *path) noexcept -> Result<Texture>;

        [[nodiscard]] auto width() const noexcept -> i32 { return m_width; }
        [[nodiscard]] auto height() const noexcept -> i32 { return m_height; }

        [[nodiscard]] auto get_pixel(i32 x, i32 y) const noexcept -> Color;
        [[nodiscard]] auto get_pixel_argb(i32 x, i32 y) const noexcept -> u32;

        [[nodiscard]] auto in_bounds(i32 x, i32 y) const noexcept -> bool {
            return x >= 0 && x < m_width && y >= 0 && y < m_height;
        }

        [[nodiscard]] auto data() const noexcept -> const u32 * { return m_buf.data(); }
        [[nodiscard]] auto stride() const noexcept -> i32 { return m_width; }

    private:
        Texture(i32 w, i32 h, std::vector<u32> buf) noexcept;

        i32 m_width;
        i32 m_height;
        std::vector<u32> m_buf; // ARGB format
    };
}
