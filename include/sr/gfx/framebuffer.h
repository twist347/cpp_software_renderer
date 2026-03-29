#pragma once

#include <span>
#include <vector>

#include "sr/core/types.h"
#include "sr/core/color.h"

namespace sr {
    class FrameBuffer {
    public:
        static Result<FrameBuffer> create(i32 width, i32 height) noexcept;


        void clear(Color c = colors::black) noexcept;

        void clear(u32 argb) noexcept;

        void set_pixel(i32 x, i32 y, Color c) noexcept;

        void set_pixel(i32 x, i32 y, u32 argb) noexcept;

        [[nodiscard]] Color get_pixel(i32 x, i32 y) const noexcept;

        void fill_hor_line(i32 x0, i32 x1, i32 y, u32 argb) noexcept;

        [[nodiscard]] bool in_bounds(i32 x, i32 y) const noexcept {
            return x >= 0 && x < m_width && y >= 0 && y < m_height;
        }

        [[nodiscard]] const u32 *data() const noexcept { return m_buf.data(); }
        [[nodiscard]] u32 *data() noexcept { return m_buf.data(); }
        [[nodiscard]] std::span<const u32> span() const noexcept { return m_buf; }

        [[nodiscard]] i32 width() const noexcept { return m_width; }
        [[nodiscard]] i32 height() const noexcept { return m_height; }
        [[nodiscard]] i32 stride() const noexcept { return m_width; }

        [[nodiscard]] Status resize(i32 w, i32 h) noexcept;

    private:
        FrameBuffer(i32 w, i32 h, std::vector<u32> buf) noexcept;

        i32 m_width;
        i32 m_height;
        std::vector<u32> m_buf;
    };
}
