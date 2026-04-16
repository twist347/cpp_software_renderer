#include "sr/gfx/framebuffer.h"

#include <algorithm>

namespace sr {
    FrameBuffer::FrameBuffer(i32 w, i32 h, std::vector<Pixel> buf) noexcept
        : m_width{w}, m_height{h}, m_buf{std::move(buf)} {
    }

    auto FrameBuffer::create(i32 width, i32 height) noexcept -> Result<FrameBuffer> {
        if (width <= 0 || height <= 0) {
            return std::unexpected{Error::InvalidDimensions};
        }
        std::vector<Pixel> buf(static_cast<usize>(width) * height, 0);
        return FrameBuffer{width, height, std::move(buf)};
    }

    auto FrameBuffer::clear(Color c) noexcept -> void {
        clear(c.to_argb());
    }

    auto FrameBuffer::clear(Pixel p) noexcept -> void {
        std::fill(m_buf.begin(), m_buf.end(), p);
    }

    auto FrameBuffer::clip_hor_line(i32 &x0, i32 &x1, i32 y) noexcept -> Pixel * {
        if (y < 0 || y >= m_height) {
            return nullptr;
        }
        if (x0 > x1) {
            std::swap(x0, x1);
        }
        x0 = std::max(x0, 0);
        x1 = std::min(x1, m_width - 1);
        if (x0 > x1) {
            return nullptr;
        }
        return m_buf.data() + static_cast<usize>(y) * m_width;
    }

    auto FrameBuffer::fill_hor_line(i32 x0, i32 x1, i32 y, Pixel p) noexcept -> void {
        if (auto *row = clip_hor_line(x0, x1, y)) {
            std::fill(row + x0, row + x1 + 1, p);
        }
    }

    auto FrameBuffer::fill_hor_line(i32 x0, i32 x1, i32 y, Color c) noexcept -> void {
        if (c.a == 0) {
            return;
        }
        if (c.a == 255) {
            fill_hor_line(x0, x1, y, c.to_argb());
            return;
        }
        if (auto *row = clip_hor_line(x0, x1, y)) {
            for (i32 x = x0; x <= x1; ++x) {
                row[x] = c.blend_over(Color::from_argb(row[x])).to_argb();
            }
        }
    }

    auto FrameBuffer::fill_hor_line_unchecked(i32 x0, i32 x1, i32 y, Pixel p) noexcept -> void {
        assert(x0 <= x1 && in_bounds(x0, y) && in_bounds(x1, y));
        auto *row = m_buf.data() + static_cast<usize>(y) * m_width;
        std::fill(row + x0, row + x1 + 1, p);
    }

    auto FrameBuffer::fill_hor_line_unchecked(i32 x0, i32 x1, i32 y, Color c) noexcept -> void {
        assert(x0 <= x1 && in_bounds(x0, y) && in_bounds(x1, y));
        if (c.a == 0) {
            return;
        }
        if (c.a == 255) {
            fill_hor_line_unchecked(x0, x1, y, c.to_argb());
            return;
        }
        auto *row = m_buf.data() + static_cast<usize>(y) * m_width;
        for (i32 x = x0; x <= x1; ++x) {
            row[x] = c.blend_over(Color::from_argb(row[x])).to_argb();
        }
    }

    auto FrameBuffer::resize(i32 w, i32 h) noexcept -> Status {
        if (w <= 0 || h <= 0) {
            return std::unexpected{Error::InvalidDimensions};
        }
        m_width = w;
        m_height = h;
        m_buf.assign(static_cast<usize>(w) * h, 0);
        return {};
    }
}
