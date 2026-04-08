#include "sr/gfx/rasterizer.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <vector>

namespace sr::raster {
    static auto blend_pixel(FrameBuffer &fb, i32 x, i32 y, Color src) noexcept -> void;

    // C++ integer division truncates toward zero; scanline rasterization needs floor division
    static constexpr auto floor_div(i32 a, i32 b) noexcept -> i32;

    // Cohen-Sutherland clipping
    static constexpr i32 CS_INSIDE = 0;
    static constexpr i32 CS_LEFT = 1;
    static constexpr i32 CS_RIGHT = 2;
    static constexpr i32 CS_BOTTOM = 4;
    static constexpr i32 CS_TOP = 8;

    static auto cs_outcode(i32 x, i32 y, i32 xmin, i32 ymin, i32 xmax, i32 ymax) noexcept -> i32;

    static auto clip_line(i32 &x0, i32 &y0, i32 &x1, i32 &y1, i32 w, i32 h) noexcept -> bool;

    // Cohen-Sutherland clip + Bresenham; all pixels guaranteed in bounds after clipping
    auto draw_line(FrameBuffer &fb, Vec2i a, Vec2i b, Color c) noexcept -> void {
        auto [x0, y0] = a;
        auto [x1, y1] = b;

        if (!clip_line(x0, y0, x1, y1, fb.width(), fb.height())) {
            return;
        }

        const i32 dx = std::abs(x1 - x0);
        const i32 dy = -std::abs(y1 - y0);
        const i32 sx = x0 < x1 ? 1 : -1;
        const i32 sy = y0 < y1 ? 1 : -1;
        i32 err = dx + dy;

        const u32 argb = c.to_argb();

        while (true) {
            fb.set_pixel_unchecked(x0, y0, argb);
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
            Vec2i{
                static_cast<i32>(static_cast<f32>(a.x()) + nx),
                static_cast<i32>(static_cast<f32>(a.y()) + ny)
            },
            Vec2i{
                static_cast<i32>(static_cast<f32>(b.x()) + nx),
                static_cast<i32>(static_cast<f32>(b.y()) + ny)
            },
            Vec2i{
                static_cast<i32>(static_cast<f32>(b.x()) - nx),
                static_cast<i32>(static_cast<f32>(b.y()) - ny)
            },
            Vec2i{
                static_cast<i32>(static_cast<f32>(a.x()) - nx),
                static_cast<i32>(static_cast<f32>(a.y()) - ny)
            },
        };

        fill_polygon(fb, vertices, c);
    }

    // pre-clipped: unchecked writes for both horizontal and vertical edges
    auto draw_rect(FrameBuffer &fb, Vec2i a, Vec2i b, Color c) noexcept -> void {
        auto [x0, y0] = a;
        auto [x1, y1] = b;
        if (x0 > x1) {
            std::swap(x0, x1);
        }
        if (y0 > y1) {
            std::swap(y0, y1);
        }

        const i32 fw = fb.width();
        const i32 fh = fb.height();

        // clip horizontal edges x-range once
        const i32 cx0 = std::max(x0, 0);
        const i32 cx1 = std::min(x1, fw - 1);
        const u32 argb = c.to_argb();

        if (cx0 <= cx1) {
            if (y0 >= 0 && y0 < fh) {
                fb.fill_hor_line_unchecked(cx0, cx1, y0, argb);
            }
            if (y1 != y0 && y1 >= 0 && y1 < fh) {
                fb.fill_hor_line_unchecked(cx0, cx1, y1, argb);
            }
        }

        // clip vertical edges y-range once
        const i32 vy0 = std::max(y0 + 1, 0);
        const i32 vy1 = std::min(y1 - 1, fh - 1);
        for (i32 y = vy0; y <= vy1; ++y) {
            if (x0 >= 0 && x0 < fw) {
                fb.set_pixel_unchecked(x0, y, argb);
            }
            if (x1 >= 0 && x1 < fw && x1 != x0) {
                fb.set_pixel_unchecked(x1, y, argb);
            }
        }
    }

    // rect clipped once; x-range constant across all scanlines → unchecked fill
    auto fill_rect(FrameBuffer &fb, Vec2i a, Vec2i b, Color c) noexcept -> void {
        auto [x0, y0] = a;
        auto [x1, y1] = b;
        if (x0 > x1) {
            std::swap(x0, x1);
        }
        if (y0 > y1) {
            std::swap(y0, y1);
        }

        x0 = std::max(x0, 0);
        y0 = std::max(y0, 0);
        x1 = std::min(x1, fb.width() - 1);
        y1 = std::min(y1, fb.height() - 1);
        if (x0 > x1 || y0 > y1) {
            return;
        }

        for (i32 y = y0; y <= y1; ++y) {
            fb.fill_hor_line_unchecked(x0, x1, y, c);
        }
    }

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

        while (x >= y) {
            fb.fill_hor_line(cx - x, cx + x, cy + y, c);
            if (y != 0) {
                fb.fill_hor_line(cx - x, cx + x, cy - y, c);
            }
            if (x != y) {
                fb.fill_hor_line(cx - y, cx + y, cy + x, c);
                fb.fill_hor_line(cx - y, cx + y, cy - x, c);
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
        const i64 rx2 = static_cast<i64>(rx) * rx;
        const i64 ry2 = static_cast<i64>(ry) * ry;
        i32 x = 0;
        i32 y = ry;
        i64 px = 0;
        i64 py = 2 * rx2 * y;

        i32 last_y = y + 1;

        const auto fill_scan_lines = [&](i32 xi, i32 yi) -> void {
            if (yi != last_y) {
                fb.fill_hor_line(cx - xi, cx + xi, cy + yi, c);
                if (yi != 0) {
                    fb.fill_hor_line(cx - xi, cx + xi, cy - yi, c);
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

        auto edge_x = [](Vec2i from, Vec2i to, i32 y) -> i32 {
            const i32 dy = to.y() - from.y();
            if (dy == 0) {
                return from.x();
            }
            return from.x() + floor_div((y - from.y()) * (to.x() - from.x()), dy);
        };

        const i32 ylo = std::max(a.y(), 0);
        const i32 yhi = std::min(c.y(), fb.height() - 1);

        // Upper half (a -> b, a -> c)
        for (i32 y = std::max(ylo, a.y()); y < std::min(b.y(), yhi + 1); ++y) {
            fb.fill_hor_line(edge_x(a, b, y), edge_x(a, c, y), y, col);
        }

        // Lower half (b -> c, a -> c)
        for (i32 y = std::max(ylo, b.y()); y <= std::min(c.y(), yhi); ++y) {
            fb.fill_hor_line(edge_x(b, c, y), edge_x(a, c, y), y, col);
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

    auto fill_polygon(FrameBuffer &fb, std::span<const Vec2i> points, Color c) noexcept -> void {
        const auto n = points.size();
        if (n < 3) {
            return;
        }

        i32 y_min = points[0].y();
        i32 y_max = points[0].y();
        for (const auto &[x, y]: points) {
            y_min = std::min(y_min, y);
            y_max = std::max(y_max, y);
        }

        y_min = std::max(y_min, 0);
        y_max = std::min(y_max, fb.height() - 1);

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

                i32 ix = ax + floor_div((y - ay) * (bx - ax), by - ay);
                intersections.push_back(ix);
            }

            std::sort(intersections.begin(), intersections.end());

            for (usize i = 0; i + 1 < intersections.size(); i += 2) {
                fb.fill_hor_line(intersections[i], intersections[i + 1], y, c);
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
                blend_pixel(fb, px + sx, py + sy, Color::from_argb(tex.get_pixel_argb_unchecked(sx, sy)));
            }
        }
    }

    auto blit_ex(
        FrameBuffer &fb, const Texture &tex, Vec2f pos, Vec2f origin, Vec2f scale, f32 angle
    ) noexcept -> void {
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

        f32 min_x = std::numeric_limits<f32>::max();
        f32 min_y = std::numeric_limits<f32>::max();
        f32 max_x = std::numeric_limits<f32>::lowest();
        f32 max_y = std::numeric_limits<f32>::lowest();
        for (const auto &corner: corners) {
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

        // incremental transform: step is constant, so 2 adds per pixel instead of full inverse transform
        const f32 step_tx = cos_a * inv_sx;
        const f32 step_ty = -sin_a * inv_sy;

        for (i32 dy = dst_y0; dy <= dst_y1; ++dy) {
            // full transform once per row
            const f32 rx0 = static_cast<f32>(dst_x0) - pos.x();
            const f32 ry = static_cast<f32>(dy) - pos.y();
            f32 tx = (rx0 * cos_a + ry * sin_a) * inv_sx + origin.x();
            f32 ty = (-rx0 * sin_a + ry * cos_a) * inv_sy + origin.y();

            for (i32 dx = dst_x0; dx <= dst_x1; ++dx) {
                const i32 itx = static_cast<i32>(std::floor(tx));
                const i32 ity = static_cast<i32>(std::floor(ty));

                if (tex.in_bounds(itx, ity)) {
                    blend_pixel(fb, dx, dy, Color::from_argb(tex.get_pixel_argb_unchecked(itx, ity)));
                }

                tx += step_tx;
                ty += step_ty;
            }
        }
    }

    static auto blend_pixel(FrameBuffer &fb, i32 x, i32 y, Color src) noexcept -> void {
        if (src.a == 0) {
            return;
        }
        if (src.a == 255) {
            fb.set_pixel_unchecked(x, y, src);
        } else {
            fb.set_pixel_unchecked(x, y, src.blend_over(fb.get_pixel_unchecked(x, y)));
        }
    }

    static constexpr auto floor_div(i32 a, i32 b) noexcept -> i32 {
        return a / b - (a % b != 0 && (a ^ b) < 0);
    }

    static auto cs_outcode(i32 x, i32 y, i32 xmin, i32 ymin, i32 xmax, i32 ymax) noexcept -> i32 {
        i32 code = CS_INSIDE;
        if (x < xmin) {
            code |= CS_LEFT;
        } else if (x > xmax) {
            code |= CS_RIGHT;
        }
        if (y < ymin) {
            code |= CS_BOTTOM;
        } else if (y > ymax) {
            code |= CS_TOP;
        }
        return code;
    }

    static auto clip_line(i32 &x0, i32 &y0, i32 &x1, i32 &y1, i32 w, i32 h) noexcept -> bool {
        const i32 xmin = 0, ymin = 0, xmax = w - 1, ymax = h - 1;
        auto c0 = cs_outcode(x0, y0, xmin, ymin, xmax, ymax);
        auto c1 = cs_outcode(x1, y1, xmin, ymin, xmax, ymax);

        while (true) {
            if (!(c0 | c1)) {
                return true;
            }
            if (c0 & c1) {
                return false;
            }

            const i32 co = c0 ? c0 : c1;
            const i32 dx = x1 - x0;
            const i32 dy = y1 - y0;
            i32 x{}, y{};

            if (co & CS_TOP) {
                x = x0 + floor_div(dx * (ymax - y0), dy);
                y = ymax;
            } else if (co & CS_BOTTOM) {
                x = x0 + floor_div(dx * (ymin - y0), dy);
                y = ymin;
            } else if (co & CS_RIGHT) {
                y = y0 + floor_div(dy * (xmax - x0), dx);
                x = xmax;
            } else {
                y = y0 + floor_div(dy * (xmin - x0), dx);
                x = xmin;
            }

            if (co == c0) {
                x0 = x;
                y0 = y;
                c0 = cs_outcode(x0, y0, xmin, ymin, xmax, ymax);
            } else {
                x1 = x;
                y1 = y;
                c1 = cs_outcode(x1, y1, xmin, ymin, xmax, ymax);
            }
        }
    }
}
