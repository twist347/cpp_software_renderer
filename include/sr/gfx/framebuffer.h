#pragma once

#include <span>
#include <vector>

#include "sr/core/types.h"
#include "sr/core/color.h"

namespace sr {
    class FrameBuffer {
    public:
        static auto create(i32 width, i32 height) noexcept -> Result<FrameBuffer>;

        auto clear(Color c = colors::black) noexcept -> void;

        auto clear(u32 argb) noexcept -> void;

        auto set_pixel(i32 x, i32 y, Color c) noexcept -> void;

        auto set_pixel(i32 x, i32 y, u32 argb) noexcept -> void;

        [[nodiscard]] auto get_pixel(i32 x, i32 y) const noexcept -> Color;

        auto fill_hor_line(i32 x0, i32 x1, i32 y, u32 argb) noexcept -> void;

        [[nodiscard]] auto in_bounds(i32 x, i32 y) const noexcept -> bool {
            return x >= 0 && x < m_width && y >= 0 && y < m_height;
        }

        [[nodiscard]] auto data() const noexcept -> const u32 * { return m_buf.data(); }
        [[nodiscard]] auto data() noexcept -> u32 * { return m_buf.data(); }
        [[nodiscard]] auto span() const noexcept -> std::span<const u32> { return m_buf; }

        [[nodiscard]] auto width() const noexcept -> i32 { return m_width; }
        [[nodiscard]] auto height() const noexcept -> i32 { return m_height; }
        [[nodiscard]] auto stride() const noexcept -> i32 { return m_width; }

        [[nodiscard]] auto resize(i32 w, i32 h) noexcept -> Status;

    private:
        FrameBuffer(i32 w, i32 h, std::vector<u32> buf) noexcept;

        i32 m_width;
        i32 m_height;
        std::vector<u32> m_buf;
    };
}
