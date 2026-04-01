#include "sr/gfx/rasterizer.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace sr::raster {
    // Bresenham's line algorithm
    auto draw_line(FrameBuffer &fb, Vec2i a, Vec2i b, Color c) noexcept -> void {
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

    auto draw_line_ex(FrameBuffer &fb, Vec2i a, Vec2i b, f32 thickness, Color c) noexcept -> void {
        if (thickness <= 1.0f) {
            draw_line(fb, a, b, c);
            return;
        }

        const f32 dx = static_cast<f32>(b.x() - a.x());
        const f32 dy = static_cast<f32>(b.y() - a.y());
        const f32 len = std::sqrt(dx * dx + dy * dy);
        if (len < 1.0f) {
            return;
        }

        const f32 half = thickness / 2.f;
        const f32 nx = -dy / len * half;
        const f32 ny = dx / len * half;

        const std::array<Vec2i, 4> vertices = {
            Vec2i{static_cast<i32>(static_cast<f32>(a.x()) + nx),
                  static_cast<i32>(static_cast<f32>(a.y()) + ny)},
            Vec2i{static_cast<i32>(static_cast<f32>(b.x()) + nx),
                  static_cast<i32>(static_cast<f32>(b.y()) + ny)},
            Vec2i{static_cast<i32>(static_cast<f32>(b.x()) - nx),
                  static_cast<i32>(static_cast<f32>(b.y()) - ny)},
            Vec2i{static_cast<i32>(static_cast<f32>(a.x()) - nx),
                  static_cast<i32>(static_cast<f32>(a.y()) - ny)},
        };

        fill_polygon(fb, vertices, c);
    }

    auto draw_rect(FrameBuffer &fb, Vec2i a, Vec2i b, Color c) noexcept -> void {
        auto [x0, y0] = a;
        auto [x1, y1] = b;
        if (x0 > x1) {
            std::swap(x0, x1);
        }
        if (y0 > y1) {
            std::swap(y0, y1);
        }

        const u32 argb = c.to_argb();
        fb.fill_hor_line(x0, x1, y0, argb);
        if (y0 == y1) {
            return;
        }
        fb.fill_hor_line(x0, x1, y1, argb);

        for (i32 y = y0 + 1; y < y1; ++y) {
            fb.set_pixel(x0, y, argb);
            fb.set_pixel(x1, y, argb);
        }
    }

    auto fill_rect(FrameBuffer &fb, Vec2i a, Vec2i b, Color c) noexcept -> void {
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
    auto draw_circle(FrameBuffer &fb, Vec2i center, i32 r, Color c) noexcept -> void {
        if (r <= 0) {
            return;
        }
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

    auto fill_circle(FrameBuffer &fb, Vec2i center, i32 r, Color c) noexcept -> void {
        if (r <= 0) {
            return;
        }
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

    // Midpoint ellipse algorithm
    auto draw_ellipse(FrameBuffer &fb, Vec2i center, i32 rx, i32 ry, Color c) noexcept -> void {
        if (rx <= 0 || ry <= 0) {
            return;
        }
        const auto [cx, cy] = center;
        const i64 rx2 = static_cast<i64>(rx) * rx;
        const i64 ry2 = static_cast<i64>(ry) * ry;
        i32 x = 0;
        i32 y = ry;
        i64 px = 0;
        i64 py = 2 * rx2 * y;

        // Region 1: dy/dx > -1
        i64 d1 = ry2 - rx2 * ry + rx2 / 4;
        while (px < py) {
            fb.set_pixel(cx + x, cy + y, c);
            fb.set_pixel(cx - x, cy + y, c);
            fb.set_pixel(cx + x, cy - y, c);
            fb.set_pixel(cx - x, cy - y, c);
            ++x;
            px += 2 * ry2;
            if (d1 < 0) {
                d1 += ry2 * (2 * x + 1);
            } else {
                --y;
                py -= 2 * rx2;
                d1 += ry2 * (2 * x + 1) - 2 * rx2 * y;
            }
        }

        // Region 2: dy/dx <= -1
        i64 d2 = ry2 * (2 * x + 1) * (2 * x + 1) / 4 + rx2 * (y - 1) * (y - 1) - rx2 * ry2;
        while (y >= 0) {
            fb.set_pixel(cx + x, cy + y, c);
            fb.set_pixel(cx - x, cy + y, c);
            fb.set_pixel(cx + x, cy - y, c);
            fb.set_pixel(cx - x, cy - y, c);
            --y;
            py -= 2 * rx2;
            if (d2 > 0) {
                d2 += rx2 * (1 - 2 * y);
            } else {
                ++x;
                px += 2 * ry2;
                d2 += ry2 * (2 * x + 1) + rx2 * (1 - 2 * y);
            }
        }
    }

    auto fill_ellipse(FrameBuffer &fb, Vec2i center, i32 rx, i32 ry, Color c) noexcept -> void {
        if (rx <= 0 || ry <= 0) {
            return;
        }
        const auto [cx, cy] = center;
        const u32 argb = c.to_argb();
        const i64 rx2 = static_cast<i64>(rx) * rx;
        const i64 ry2 = static_cast<i64>(ry) * ry;
        i32 x = 0;
        i32 y = ry;
        i64 px = 0;
        i64 py = 2 * rx2 * y;

        i32 last_y = y + 1;

        const auto fill_scan_lines = [&](i32 xi, i32 yi) -> void {
            if (yi != last_y) {
                fb.fill_hor_line(cx - xi, cx + xi, cy + yi, argb);
                if (yi != 0) {
                    fb.fill_hor_line(cx - xi, cx + xi, cy - yi, argb);
                }
                last_y = yi;
            }
        };

        // Region 1
        i64 d1 = ry2 - rx2 * ry + rx2 / 4;
        while (px < py) {
            fill_scan_lines(x, y);
            ++x;
            px += 2 * ry2;
            if (d1 < 0) {
                d1 += ry2 * (2 * x + 1);
            } else {
                --y;
                py -= 2 * rx2;
                d1 += ry2 * (2 * x + 1) - 2 * rx2 * y;
            }
        }

        // Region 2
        i64 d2 = ry2 * (2 * x + 1) * (2 * x + 1) / 4 + rx2 * (y - 1) * (y - 1) - rx2 * ry2;
        while (y >= 0) {
            fill_scan_lines(x, y);
            --y;
            py -= 2 * rx2;
            if (d2 > 0) {
                d2 += rx2 * (1 - 2 * y);
            } else {
                ++x;
                px += 2 * ry2;
                d2 += ry2 * (2 * x + 1) + rx2 * (1 - 2 * y);
            }
        }
    }

    auto draw_triangle(FrameBuffer &fb, Vec2i a, Vec2i b, Vec2i c, Color col) noexcept -> void {
        draw_line(fb, a, b, col);
        draw_line(fb, b, c, col);
        draw_line(fb, c, a, col);
    }

    // Scanline triangle fill
    auto fill_triangle(FrameBuffer &fb, Vec2i a, Vec2i b, Vec2i c, Color col) noexcept -> void {
        if (a.y() > b.y()) {
            std::swap(a, b);
        }
        if (a.y() > c.y()) {
            std::swap(a, c);
        }
        if (b.y() > c.y()) {
            std::swap(b, c);
        }

        if (a.y() == c.y()) {
            return; // degenerate
        }

        const u32 argb = col.to_argb();

        auto edge_x = [](Vec2i from, Vec2i to, i32 y) -> i32 {
            const i32 dy = to.y() - from.y();
            if (dy == 0) {
                return from.x();
            }
            return from.x() + (y - from.y()) * (to.x() - from.x()) / dy;
        };

        // Upper half (a -> b, a -> c)
        for (i32 y = a.y(); y < b.y(); ++y) {
            fb.fill_hor_line(edge_x(a, b, y), edge_x(a, c, y), y, argb);
        }

        // Lower half (b -> c, a -> c)
        if (b.y() == c.y()) {
            fb.fill_hor_line(b.x(), c.x(), b.y(), argb);
        } else {
            for (i32 y = b.y(); y <= c.y(); ++y) {
                fb.fill_hor_line(edge_x(b, c, y), edge_x(a, c, y), y, argb);
            }
        }
    }

    auto draw_polygon(FrameBuffer &fb, std::span<const Vec2i> points, Color c) noexcept -> void {
        const auto n = points.size();
        if (n < 2) {
            return;
        }
        for (usize i = 0; i < n; ++i) {
            draw_line(fb, points[i], points[(i + 1) % n], c);
        }
    }

    // Scanline fill with sorted edge intersections (even-odd rule)
    auto fill_polygon(FrameBuffer &fb, std::span<const Vec2i> points, Color c) noexcept -> void {
        const auto n = points.size();
        if (n < 3) {
            return;
        }

        i32 y_min = points[0].y();
        i32 y_max = points[0].y();
        for (const auto &[x, y] : points) {
            y_min = std::min(y_min, y);
            y_max = std::max(y_max, y);
        }

        const u32 argb = c.to_argb();
        thread_local std::vector<i32> intersections;

        for (i32 y = y_min; y <= y_max; ++y) {
            intersections.clear();

            for (usize i = 0; i < n; ++i) {
                const auto &[ax, ay] = points[i];
                const auto &[bx, by] = points[(i + 1) % n];

                if (ay == by) {
                    continue;
                }

                // Edge crosses scanline if y is in [min_y, max_y) — exclude top endpoint to avoid double-counting at vertices
                const i32 lo = std::min(ay, by);
                if (const i32 hi = std::max(ay, by); y < lo || y >= hi) {
                    continue;
                }

                i32 ix = ax + (y - ay) * (bx - ax) / (by - ay);
                intersections.push_back(ix);
            }

            std::sort(intersections.begin(), intersections.end());

            for (usize i = 0; i + 1 < intersections.size(); i += 2) {
                fb.fill_hor_line(intersections[i], intersections[i + 1], y, argb);
            }
        }
    }

    auto blit(FrameBuffer &fb, const Texture &tex, Vec2i pos) noexcept -> void {
        const auto [px, py] = pos;
        const i32 tw = tex.width();
        const i32 th = tex.height();

        const i32 x0 = std::max(0, -px);
        const i32 y0 = std::max(0, -py);
        const i32 x1 = std::min(tw, fb.width() - px);
        const i32 y1 = std::min(th, fb.height() - py);
        if (x0 >= x1 || y0 >= y1) {
            return;
        }

        for (i32 sy = y0; sy < y1; ++sy) {
            for (i32 sx = x0; sx < x1; ++sx) {
                const auto src = Color::from_argb(tex.get_pixel_argb(sx, sy));
                if (src.a == 0) {
                    continue;
                }
                if (src.a == 255) {
                    fb.set_pixel(px + sx, py + sy, src);
                } else {
                    fb.set_pixel(px + sx, py + sy, src.blend_over(fb.get_pixel(px + sx, py + sy)));
                }
            }
        }
    }

    auto blit_ex(FrameBuffer &fb, const Texture &tex, Vec2f pos, Vec2f origin, Vec2f scale, f32 angle) noexcept -> void {
        const f32 sx = scale.x();
        const f32 sy = scale.y();
        if (sx == 0.f || sy == 0.f) {
            return;
        }

        const f32 sin_a = std::sin(angle);
        const f32 cos_a = std::cos(angle);
        const f32 tw = static_cast<f32>(tex.width());
        const f32 th = static_cast<f32>(tex.height());

        // compute AABB of the transformed sprite to iterate only relevant pixels
        const f32 hw = tw * std::abs(sx);
        const f32 hh = th * std::abs(sy);
        const std::array<Vec2f, 4> corners = {
            Vec2f{0.f, 0.f}, Vec2f{hw, 0.f}, Vec2f{hw, hh}, Vec2f{0.f, hh}
        };

        f32 min_x = 1e9f, min_y = 1e9f, max_x = -1e9f, max_y = -1e9f;
        for (const auto &corner : corners) {
            const f32 dx = corner.x() - origin.x() * std::abs(sx);
            const f32 dy = corner.y() - origin.y() * std::abs(sy);
            const f32 rx = pos.x() + dx * cos_a - dy * sin_a;
            const f32 ry = pos.y() + dx * sin_a + dy * cos_a;
            min_x = std::min(min_x, rx);
            min_y = std::min(min_y, ry);
            max_x = std::max(max_x, rx);
            max_y = std::max(max_y, ry);
        }

        // clamp to framebuffer
        const i32 dst_x0 = std::max(0, static_cast<i32>(min_x));
        const i32 dst_y0 = std::max(0, static_cast<i32>(min_y));
        const i32 dst_x1 = std::min(fb.width() - 1, static_cast<i32>(max_x) + 1);
        const i32 dst_y1 = std::min(fb.height() - 1, static_cast<i32>(max_y) + 1);

        // inverse transform: screen -> texture
        const f32 inv_sx = 1.f / sx;
        const f32 inv_sy = 1.f / sy;

        for (i32 dy = dst_y0; dy <= dst_y1; ++dy) {
            for (i32 dx = dst_x0; dx <= dst_x1; ++dx) {
                // rotate back
                const f32 rx = static_cast<f32>(dx) - pos.x();
                const f32 ry = static_cast<f32>(dy) - pos.y();
                const f32 lx = rx * cos_a + ry * sin_a;
                const f32 ly = -rx * sin_a + ry * cos_a;

                // unscale + re-add origin
                const f32 tx = lx * inv_sx + origin.x();
                const f32 ty = ly * inv_sy + origin.y();

                const i32 itx = static_cast<i32>(std::floor(tx));
                const i32 ity = static_cast<i32>(std::floor(ty));

                if (tex.in_bounds(itx, ity)) {
                    const auto src = Color::from_argb(tex.get_pixel_argb(itx, ity));
                    if (src.a == 0) {
                        continue;
                    }
                    if (src.a == 255) {
                        fb.set_pixel(dx, dy, src);
                    } else {
                        fb.set_pixel(dx, dy, src.blend_over(fb.get_pixel(dx, dy)));
                    }
                }
            }
        }
    }
}
