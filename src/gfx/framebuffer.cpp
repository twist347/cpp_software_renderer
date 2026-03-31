#include "sr/gfx/framebuffer.h"

#include <algorithm>

namespace sr {
    FrameBuffer::FrameBuffer(i32 w, i32 h, std::vector<u32> buf) noexcept
        : m_width{w}, m_height{h}, m_buf{std::move(buf)} {
    }

    auto FrameBuffer::create(i32 width, i32 height) noexcept -> Result<FrameBuffer> {
        if (width <= 0 || height <= 0) {
            return std::unexpected{Error::InvalidDimensions};
        }
        std::vector<u32> buf(static_cast<std::size_t>(width) * height, 0);
        return FrameBuffer{width, height, std::move(buf)};
    }

    auto FrameBuffer::clear(Color c) noexcept -> void {
        clear(c.to_argb());
    }

    auto FrameBuffer::clear(u32 argb) noexcept -> void {
        std::fill(m_buf.begin(), m_buf.end(), argb);
    }

    auto FrameBuffer::set_pixel(i32 x, i32 y, Color c) noexcept -> void {
        set_pixel(x, y, c.to_argb());
    }

    auto FrameBuffer::set_pixel(i32 x, i32 y, u32 argb) noexcept -> void {
        if (in_bounds(x, y)) {
            m_buf[static_cast<std::size_t>(y) * m_width + x] = argb;
        }
    }

    auto FrameBuffer::get_pixel(i32 x, i32 y) const noexcept -> Color {
        if (in_bounds(x, y)) {
            return Color::from_argb(m_buf[static_cast<std::size_t>(y) * m_width + x]);
        }
        return {};
    }

    auto FrameBuffer::fill_hor_line(i32 x0, i32 x1, i32 y, u32 argb) noexcept -> void {
        if (y < 0 || y >= m_height) {
            return;
        }
        if (x0 > x1) {
            std::swap(x0, x1);
        }

        // clip
        x0 = std::max(x0, 0);
        x1 = std::min(x1, m_width - 1);
        if (x0 > x1) {
            return;
        }

        auto *row = m_buf.data() + static_cast<std::size_t>(y) * m_width;
        std::fill(row + x0, row + x1 + 1, argb);
    }

    auto FrameBuffer::resize(i32 w, i32 h) noexcept -> Status {
        if (w <= 0 || h <= 0) {
            return std::unexpected{Error::InvalidDimensions};
        }
        m_width = w;
        m_height = h;
        m_buf.assign(static_cast<std::size_t>(w) * h, 0);
        return {};
    }
}
