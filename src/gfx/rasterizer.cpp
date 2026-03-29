#include "sr/gfx/rasterizer.h"

#include <algorithm>

namespace sr::raster {
    void put_pixel(FrameBuffer &fb, i32 x, i32 y, Color c) noexcept {
        fb.set_pixel(x, y, c);
    }

    // Bresenham's line algorithm
    void line(FrameBuffer &fb, Vec2i a, Vec2i b, Color c) noexcept {
        auto [x0, y0] = a;
        const auto [x1, y1] = b;

        const i32 dx = std::abs(x1 - x0);
        const i32 dy = -std::abs(y1 - y0);
        const i32 sx = x0 < x1 ? 1 : -1;
        const i32 sy = y0 < y1 ? 1 : -1;
        i32 err = dx + dy;

        while (true) {
            fb.set_pixel(x0, y0, c);
            if (x0 == x1 && y0 == y1) {
                break;
            }
            const i32 e2 = 2 * err;
            if (e2 >= dy) {
                err += dy;
                x0 += sx;
            }
            if (e2 <= dx) {
                err += dx;
                y0 += sy;
            }
        }
    }

    void draw_rect(FrameBuffer &fb, Vec2i a, Vec2i b, Color c) noexcept {
        auto [x0, y0] = a;
        auto [x1, y1] = b;
        if (x0 > x1) {
            std::swap(x0, x1);
        }
        if (y0 > y1) {
            std::swap(y0, y1);
        }

        for (i32 x = x0; x <= x1; ++x) {
            fb.set_pixel(x, y0, c);
            fb.set_pixel(x, y1, c);
        }
        for (i32 y = y0 + 1; y < y1; ++y) {
            fb.set_pixel(x0, y, c);
            fb.set_pixel(x1, y, c);
        }
    }

    void fill_rect(FrameBuffer &fb, Vec2i a, Vec2i b, Color c) noexcept {
        auto [x0, y0] = a;
        auto [x1, y1] = b;
        if (x0 > x1) {
            std::swap(x0, x1);
        }
        if (y0 > y1) {
            std::swap(y0, y1);
        }

        const u32 argb = c.to_argb();
        for (i32 y = y0; y <= y1; ++y) {
            fb.fill_hor_line(x0, x1, y, argb);
        }
    }

    // Midpoint circle algorithm
    void draw_circle(FrameBuffer &fb, Vec2i center, i32 r, Color c) noexcept {
        const auto [cx, cy] = center;
        i32 x = r;
        i32 y = 0;
        i32 err = 1 - x;

        while (x >= y) {
            fb.set_pixel(cx + x, cy + y, c);
            fb.set_pixel(cx - x, cy + y, c);
            fb.set_pixel(cx + x, cy - y, c);
            fb.set_pixel(cx - x, cy - y, c);
            fb.set_pixel(cx + y, cy + x, c);
            fb.set_pixel(cx - y, cy + x, c);
            fb.set_pixel(cx + y, cy - x, c);
            fb.set_pixel(cx - y, cy - x, c);

            ++y;
            if (err < 0) {
                err += 2 * y + 1;
            } else {
                --x;
                err += 2 * (y - x) + 1;
            }
        }
    }

    void fill_circle(FrameBuffer &fb, Vec2i center, i32 r, Color c) noexcept {
        const auto [cx, cy] = center;
        i32 x = r;
        i32 y = 0;
        i32 err = 1 - x;
        const u32 argb = c.to_argb();

        while (x >= y) {
            fb.fill_hor_line(cx - x, cx + x, cy + y, argb);
            if (y != 0) {
                fb.fill_hor_line(cx - x, cx + x, cy - y, argb);
            }
            if (x != y) {
                fb.fill_hor_line(cx - y, cx + y, cy + x, argb);
                fb.fill_hor_line(cx - y, cx + y, cy - x, argb);
            }

            ++y;
            if (err < 0) {
                err += 2 * y + 1;
            } else {
                --x;
                err += 2 * (y - x) + 1;
            }
        }
    }

    // Scanline triangle fill
    void fill_triangle(FrameBuffer &fb, Vec2i a, Vec2i b, Vec2i c, Color col) noexcept {
        if (a.y > b.y) {
            std::swap(a, b);
        }
        if (a.y > c.y) {
            std::swap(a, c);
        }
        if (b.y > c.y) {
            std::swap(b, c);
        }

        if (a.y == c.y) {
            return; // degenerate
        }

        const u32 argb = col.to_argb();

        auto edge_x = [](Vec2i from, Vec2i to, i32 y) -> i32 {
            const i32 dy = to.y - from.y;
            if (dy == 0) {
                return from.x;
            }
            return from.x + (y - from.y) * (to.x - from.x) / dy;
        };

        // Upper half (a -> b, a -> c)
        for (i32 y = a.y; y < b.y; ++y) {
            fb.fill_hor_line(edge_x(a, b, y), edge_x(a, c, y), y, argb);
        }

        // Lower half (b -> c, a -> c)
        if (b.y == c.y) {
            fb.fill_hor_line(b.x, c.x, b.y, argb);
        } else {
            for (i32 y = b.y; y <= c.y; ++y) {
                fb.fill_hor_line(edge_x(b, c, y), edge_x(a, c, y), y, argb);
            }
        }
    }
}
