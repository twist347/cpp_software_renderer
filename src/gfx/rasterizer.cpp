#include "sr/gfx/rasterizer.h"

#include "sr/core/macros.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

namespace sr::raster {
    namespace {
        // -------- runtime enum → compile-time template dispatchers --------
        // each invokes f(std::integral_constant<Enum, Value>{}); the lambda captures

        template<typename F>
        SR_INLINE auto dispatch_blend(BlendMode m, F &&f) noexcept -> void {
            switch (m) {
                case BlendMode::None:
                    std::forward<F>(f)(std::integral_constant<BlendMode, BlendMode::None>{});
                    return;
                case BlendMode::Alpha:
                    std::forward<F>(f)(std::integral_constant<BlendMode, BlendMode::Alpha>{});
                    return;
                case BlendMode::Additive:
                    std::forward<F>(f)(std::integral_constant<BlendMode, BlendMode::Additive>{});
            }
        }

        template<typename F>
        SR_INLINE auto dispatch_filter(SamplerFilter s, F &&f) noexcept -> void {
            switch (s) {
                case SamplerFilter::Nearest:
                    std::forward<F>(f)(std::integral_constant<SamplerFilter, SamplerFilter::Nearest>{});
                    return;
                case SamplerFilter::Bilinear:
                    std::forward<F>(f)(std::integral_constant<SamplerFilter, SamplerFilter::Bilinear>{});
            }
        }

        template<typename F>
        SR_INLINE auto dispatch_wrap(WrapMode w, F &&f) noexcept -> void {
            switch (w) {
                case WrapMode::Clamp:
                    std::forward<F>(f)(std::integral_constant<WrapMode, WrapMode::Clamp>{});
                    return;
                case WrapMode::Repeat:
                    std::forward<F>(f)(std::integral_constant<WrapMode, WrapMode::Repeat>{});
                    return;
                case WrapMode::Mirror:
                    std::forward<F>(f)(std::integral_constant<WrapMode, WrapMode::Mirror>{});
            }
        }

        // -------- compile-time pixel-write dispatchers --------

        template<BlendMode M>
        SR_INLINE auto write_pixel(FrameBuffer &fb, i32 x, i32 y, Color src) noexcept -> void {
            if constexpr (M == BlendMode::None) {
                fb.set_pixel_unchecked(x, y, src);
            } else {
                if (src.a == 0) {
                    return;
                }
                if constexpr (M == BlendMode::Alpha) {
                    if (src.a == 255) {
                        fb.set_pixel_unchecked(x, y, src);
                    } else {
                        fb.set_pixel_unchecked(x, y, src.blend_over(fb.get_pixel_unchecked(x, y)));
                    }
                } else {
                    // Additive
                    const Color dst = fb.get_pixel_unchecked(x, y);
                    const u32 sa = src.a;
                    fb.set_pixel_unchecked(
                        x, y, Color{
                            static_cast<u8>(std::min(255u, dst.r + (src.r * sa + 127) / 255)),
                            static_cast<u8>(std::min(255u, dst.g + (src.g * sa + 127) / 255)),
                            static_cast<u8>(std::min(255u, dst.b + (src.b * sa + 127) / 255)),
                            dst.a,
                        }
                    );
                }
            }
        }

        template<BlendMode M>
        SR_INLINE auto write_scanline(FrameBuffer &fb, i32 x0, i32 x1, i32 y, Color src) noexcept -> void {
            if constexpr (M == BlendMode::None) {
                fb.fill_hor_line_unchecked(x0, x1, y, src.to_argb());
            } else {
                if (src.a == 0) {
                    return;
                }
                if constexpr (M == BlendMode::Alpha) {
                    fb.fill_hor_line_unchecked(x0, x1, y, src);
                } else {
                    // Additive
                    const u32 sa = src.a;
                    const u32 add_r = (src.r * sa + 127) / 255;
                    const u32 add_g = (src.g * sa + 127) / 255;
                    const u32 add_b = (src.b * sa + 127) / 255;
                    for (i32 x = x0; x <= x1; ++x) {
                        const Color dst = fb.get_pixel_unchecked(x, y);
                        fb.set_pixel_unchecked(
                            x, y, Color{
                                static_cast<u8>(std::min(255u, dst.r + add_r)),
                                static_cast<u8>(std::min(255u, dst.g + add_g)),
                                static_cast<u8>(std::min(255u, dst.b + add_b)),
                                dst.a,
                            }
                        );
                    }
                }
            }
        }

        // -------- compile-time sampler --------

        template<WrapMode W>
        SR_INLINE auto wrap_coord(i32 v, i32 dim) noexcept -> i32 {
            if constexpr (W == WrapMode::Clamp) {
                return std::clamp(v, 0, dim - 1);
            } else if constexpr (W == WrapMode::Repeat) {
                i32 m = v % dim;
                if (m < 0) {
                    m += dim;
                }
                return m;
            } else {
                // Mirror
                const i32 p = 2 * dim;
                i32 m = v % p;
                if (m < 0) {
                    m += p;
                }
                return m < dim ? m : p - 1 - m;
            }
        }

        template<SamplerFilter F, WrapMode W>
        SR_INLINE auto sample(const Texture &tex, f32 tx, f32 ty) noexcept -> Color {
            const i32 tw = tex.width();
            const i32 th = tex.height();

            const auto fetch = [&](i32 xi, i32 yi) -> Color {
                if constexpr (W == WrapMode::Clamp) {
                    if (!tex.in_bounds(xi, yi)) {
                        return colors::transparent;
                    }
                }
                const i32 x = wrap_coord<W>(xi, tw);
                const i32 y = wrap_coord<W>(yi, th);
                return Color::from_argb(tex.get_pixel_argb_unchecked(x, y));
            };

            if constexpr (F == SamplerFilter::Nearest) {
                return fetch(static_cast<i32>(std::floor(tx)), static_cast<i32>(std::floor(ty)));
            } else {
                // Bilinear: sample at texel centers — subtract 0.5 to align
                const f32 fx = tx - 0.5f;
                const f32 fy = ty - 0.5f;
                const i32 x0 = static_cast<i32>(std::floor(fx));
                const i32 y0 = static_cast<i32>(std::floor(fy));
                const f32 rx = fx - static_cast<f32>(x0);
                const f32 ry = fy - static_cast<f32>(y0);
                const Color c00 = fetch(x0, y0);
                const Color c10 = fetch(x0 + 1, y0);
                const Color c01 = fetch(x0, y0 + 1);
                const Color c11 = fetch(x0 + 1, y0 + 1);
                return Color::lerp(Color::lerp(c00, c10, rx), Color::lerp(c01, c11, rx), ry);
            }
        }

        // -------- non-templated helpers --------

        // C++ integer division truncates toward zero; scanline rasterization needs floor division
        constexpr auto floor_div(i32 a, i32 b) noexcept -> i32 {
            return a / b - (a % b != 0 && (a ^ b) < 0);
        }

        // Cohen-Sutherland clipping
        constexpr i32 CS_INSIDE = 0;
        constexpr i32 CS_LEFT = 1;
        constexpr i32 CS_RIGHT = 2;
        constexpr i32 CS_BOTTOM = 4;
        constexpr i32 CS_TOP = 8;

        auto cs_outcode(i32 x, i32 y, i32 xmin, i32 ymin, i32 xmax, i32 ymax) noexcept -> i32 {
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

        auto clip_line(i32 &x0, i32 &y0, i32 &x1, i32 &y1, i32 w, i32 h) noexcept -> bool {
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

        // -------- shared walkers / helpers --------

        // Midpoint circle: yields each first-octant (x, y) pair exactly once.
        template<typename Visitor>
        SR_INLINE auto walk_circle_octants(i32 r, Visitor &&v) noexcept -> void {
            i32 x = r;
            i32 y = 0;
            i32 err = 1 - x;
            while (x >= y) {
                v(x, y);
                ++y;
                if (err < 0) {
                    err += 2 * y + 1;
                } else {
                    --x;
                    err += 2 * (y - x) + 1;
                }
            }
        }

        // Midpoint ellipse: yields each first-quadrant (x, y) pair from both regions.
        template<typename Visitor>
        SR_INLINE auto walk_ellipse_quadrants(i32 rx, i32 ry, Visitor &&v) noexcept -> void {
            const i64 rx2 = static_cast<i64>(rx) * rx;
            const i64 ry2 = static_cast<i64>(ry) * ry;
            i32 x = 0;
            i32 y = ry;
            i64 px = 0;
            i64 py = 2 * rx2 * y;

            // Region 1: dy/dx > -1
            i64 d1 = ry2 - rx2 * ry + rx2 / 4;
            while (px < py) {
                v(x, y);
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
                v(x, y);
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

        // Clip x-range to framebuffer and emit a scanline (y assumed in-bounds).
        template<BlendMode M>
        SR_INLINE auto emit_clipped_scanline(FrameBuffer &fb, i32 x0, i32 x1, i32 y, Color c) noexcept -> void {
            if (x0 > x1) {
                std::swap(x0, x1);
            }
            x0 = std::max(x0, 0);
            x1 = std::min(x1, fb.width() - 1);
            if (x0 <= x1) {
                write_scanline<M>(fb, x0, x1, y, c);
            }
        }

        // Sort two corners into (lo, hi) axis-aligned pair.
        SR_INLINE auto normalize_rect(Vec2i a, Vec2i b) noexcept -> std::pair<Vec2i, Vec2i> {
            return {
                Vec2i{std::min(a.x(), b.x()), std::min(a.y(), b.y())},
                Vec2i{std::max(a.x(), b.x()), std::max(a.y(), b.y())},
            };
        }

        // -------- templated primitive impls --------

        template<BlendMode M>
        auto draw_line_impl(FrameBuffer &fb, Vec2i a, Vec2i b, Color c) noexcept -> void {
            auto [x0, y0] = a;
            auto [x1, y1] = b;

            if (!clip_line(x0, y0, x1, y1, fb.width(), fb.height())) {
                return;
            }
            if constexpr (M != BlendMode::None) {
                if (c.a == 0) {
                    return;
                }
            }

            const i32 dx = std::abs(x1 - x0);
            const i32 dy = -std::abs(y1 - y0);
            const i32 sx = x0 < x1 ? 1 : -1;
            const i32 sy = y0 < y1 ? 1 : -1;
            i32 err = dx + dy;

            while (true) {
                write_pixel<M>(fb, x0, y0, c);
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

        template<BlendMode M>
        auto fill_polygon_impl(FrameBuffer &fb, std::span<const Vec2i> points, Color c) noexcept -> void;

        template<BlendMode M>
        auto draw_line_ex_impl(FrameBuffer &fb, Vec2i a, Vec2i b, f32 thickness, Color c) noexcept -> void {
            if (thickness <= 1.0f) {
                draw_line_impl<M>(fb, a, b, c);
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
                    static_cast<i32>(std::lround(static_cast<f32>(a.x()) + nx)),
                    static_cast<i32>(std::lround(static_cast<f32>(a.y()) + ny))
                },
                Vec2i{
                    static_cast<i32>(std::lround(static_cast<f32>(b.x()) + nx)),
                    static_cast<i32>(std::lround(static_cast<f32>(b.y()) + ny))
                },
                Vec2i{
                    static_cast<i32>(std::lround(static_cast<f32>(b.x()) - nx)),
                    static_cast<i32>(std::lround(static_cast<f32>(b.y()) - ny))
                },
                Vec2i{
                    static_cast<i32>(std::lround(static_cast<f32>(a.x()) - nx)),
                    static_cast<i32>(std::lround(static_cast<f32>(a.y()) - ny))
                },
            };

            fill_polygon_impl<M>(fb, vertices, c);
        }

        template<BlendMode M>
        auto draw_rect_impl(FrameBuffer &fb, Vec2i a, Vec2i b, Color c) noexcept -> void {
            if constexpr (M != BlendMode::None) {
                if (c.a == 0) {
                    return;
                }
            }

            const auto [lo, hi] = normalize_rect(a, b);
            auto [x0, y0] = lo;
            auto [x1, y1] = hi;

            const i32 fw = fb.width();
            const i32 fh = fb.height();

            if (const i32 cx0 = std::max(x0, 0), cx1 = std::min(x1, fw - 1); cx0 <= cx1) {
                if (y0 >= 0 && y0 < fh) {
                    write_scanline<M>(fb, cx0, cx1, y0, c);
                }
                if (y1 != y0 && y1 >= 0 && y1 < fh) {
                    write_scanline<M>(fb, cx0, cx1, y1, c);
                }
            }

            const i32 vy0 = std::max(y0 + 1, 0);
            const i32 vy1 = std::min(y1 - 1, fh - 1);
            const bool x0_in = x0 >= 0 && x0 < fw;
            const bool x1_in = x1 >= 0 && x1 < fw && x1 != x0;
            for (i32 y = vy0; y <= vy1; ++y) {
                if (x0_in) {
                    write_pixel<M>(fb, x0, y, c);
                }
                if (x1_in) {
                    write_pixel<M>(fb, x1, y, c);
                }
            }
        }

        template<BlendMode M>
        auto fill_rect_impl(FrameBuffer &fb, Vec2i a, Vec2i b, Color c) noexcept -> void {
            if constexpr (M != BlendMode::None) {
                if (c.a == 0) {
                    return;
                }
            }

            const auto [lo, hi] = normalize_rect(a, b);
            i32 x0 = std::max(lo.x(), 0);
            i32 y0 = std::max(lo.y(), 0);
            i32 x1 = std::min(hi.x(), fb.width() - 1);
            i32 y1 = std::min(hi.y(), fb.height() - 1);
            if (x0 > x1 || y0 > y1) {
                return;
            }

            for (i32 y = y0; y <= y1; ++y) {
                write_scanline<M>(fb, x0, x1, y, c);
            }
        }

        template<BlendMode M>
        auto draw_circle_impl(FrameBuffer &fb, Vec2i center, i32 r, Color c) noexcept -> void {
            if (r <= 0) {
                return;
            }
            if constexpr (M != BlendMode::None) {
                if (c.a == 0) {
                    return;
                }
            }
            const auto [cx, cy] = center;
            const auto put = [&](i32 px, i32 py) noexcept {
                if (fb.in_bounds(px, py)) {
                    write_pixel<M>(fb, px, py, c);
                }
            };

            walk_circle_octants(r, [&](i32 x, i32 y) noexcept {
                // 8-way symmetry with dedup at y=0 and x=y (octant boundaries)
                put(cx + x, cy + y);
                put(cx - x, cy + y);
                if (y != 0) {
                    put(cx + x, cy - y);
                    put(cx - x, cy - y);
                }
                if (x != y) {
                    put(cx + y, cy + x);
                    put(cx + y, cy - x);
                    if (y != 0) {
                        put(cx - y, cy + x);
                        put(cx - y, cy - x);
                    }
                }
            });
        }

        template<BlendMode M>
        auto fill_circle_impl(FrameBuffer &fb, Vec2i center, i32 r, Color c) noexcept -> void {
            if (r <= 0) {
                return;
            }
            if constexpr (M != BlendMode::None) {
                if (c.a == 0) {
                    return;
                }
            }
            const auto [cx, cy] = center;
            const i32 fh = fb.height();

            const auto scan = [&](i32 xa, i32 xb, i32 yy) noexcept {
                if (yy < 0 || yy >= fh) {
                    return;
                }
                emit_clipped_scanline<M>(fb, xa, xb, yy, c);
            };

            walk_circle_octants(r, [&](i32 x, i32 y) noexcept {
                scan(cx - x, cx + x, cy + y);
                if (y != 0) {
                    scan(cx - x, cx + x, cy - y);
                }
                if (x != y) {
                    scan(cx - y, cx + y, cy + x);
                    scan(cx - y, cx + y, cy - x);
                }
            });
        }

        template<BlendMode M>
        auto draw_ellipse_impl(FrameBuffer &fb, Vec2i center, i32 rx, i32 ry, Color c) noexcept -> void {
            if (rx <= 0 || ry <= 0) {
                return;
            }
            if constexpr (M != BlendMode::None) {
                if (c.a == 0) {
                    return;
                }
            }
            const auto [cx, cy] = center;
            const auto put = [&](i32 xx, i32 yy) noexcept {
                if (fb.in_bounds(xx, yy)) {
                    write_pixel<M>(fb, xx, yy, c);
                }
            };

            walk_ellipse_quadrants(rx, ry, [&](i32 xi, i32 yi) noexcept {
                // 4-way symmetry with dedup at x=0 and y=0
                put(cx + xi, cy + yi);
                if (xi != 0) {
                    put(cx - xi, cy + yi);
                }
                if (yi != 0) {
                    put(cx + xi, cy - yi);
                    if (xi != 0) {
                        put(cx - xi, cy - yi);
                    }
                }
            });
        }

        template<BlendMode M>
        auto fill_ellipse_impl(FrameBuffer &fb, Vec2i center, i32 rx, i32 ry, Color c) noexcept -> void {
            if (rx <= 0 || ry <= 0) {
                return;
            }
            if constexpr (M != BlendMode::None) {
                if (c.a == 0) {
                    return;
                }
            }
            const auto [cx, cy] = center;
            const i32 fh = fb.height();

            // skip repeated scanlines at same y (walker yields multiple (x, y) with y fixed while x grows)
            i32 last_y = ry + 1;
            const auto scan = [&](i32 xa, i32 xb, i32 yy) noexcept {
                if (yy < 0 || yy >= fh) {
                    return;
                }
                emit_clipped_scanline<M>(fb, xa, xb, yy, c);
            };

            walk_ellipse_quadrants(rx, ry, [&](i32 xi, i32 yi) noexcept {
                if (yi != last_y) {
                    scan(cx - xi, cx + xi, cy + yi);
                    if (yi != 0) {
                        scan(cx - xi, cx + xi, cy - yi);
                    }
                    last_y = yi;
                }
            });
        }

        template<BlendMode M>
        auto draw_triangle_impl(FrameBuffer &fb, Vec2i a, Vec2i b, Vec2i c, Color col) noexcept -> void {
            draw_line_impl<M>(fb, a, b, col);
            draw_line_impl<M>(fb, b, c, col);
            draw_line_impl<M>(fb, c, a, col);
        }

        template<BlendMode M>
        auto fill_triangle_impl(FrameBuffer &fb, Vec2i a, Vec2i b, Vec2i c, Color col) noexcept -> void {
            if constexpr (M != BlendMode::None) {
                if (col.a == 0) {
                    return;
                }
            }

            if (a.y() > b.y()) { std::swap(a, b); }
            if (a.y() > c.y()) { std::swap(a, c); }
            if (b.y() > c.y()) { std::swap(b, c); }

            if (a.y() == c.y()) {
                return;
            }

            const auto edge_x = [](Vec2i from, Vec2i to, i32 y) -> i32 {
                const i32 dy = to.y() - from.y();
                if (dy == 0) {
                    return from.x();
                }
                return from.x() + floor_div((y - from.y()) * (to.x() - from.x()), dy);
            };

            const i32 fh = fb.height();

            const i32 u_lo = std::max(a.y(), 0);
            const i32 u_hi = std::min(b.y(), fh);
            for (i32 y = u_lo; y < u_hi; ++y) {
                emit_clipped_scanline<M>(fb, edge_x(a, b, y), edge_x(a, c, y), y, col);
            }

            const i32 l_lo = std::max(b.y(), 0);
            const i32 l_hi = std::min(c.y(), fh - 1);
            for (i32 y = l_lo; y <= l_hi; ++y) {
                emit_clipped_scanline<M>(fb, edge_x(b, c, y), edge_x(a, c, y), y, col);
            }
        }

        template<BlendMode M>
        auto draw_polygon_impl(FrameBuffer &fb, std::span<const Vec2i> points, Color c) noexcept -> void {
            const auto n = points.size();
            if (n < 2) {
                return;
            }
            for (usize i = 0; i < n; ++i) {
                draw_line_impl<M>(fb, points[i], points[(i + 1) % n], c);
            }
        }

        template<BlendMode M>
        auto fill_polygon_impl(FrameBuffer &fb, std::span<const Vec2i> points, Color c) noexcept -> void {
            if constexpr (M != BlendMode::None) {
                if (c.a == 0) {
                    return;
                }
            }

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
                    if (const i32 lo = std::min(ay, by), hi = std::max(ay, by); y < lo || y >= hi) {
                        continue;
                    }

                    const i32 ix = ax + floor_div((y - ay) * (bx - ax), by - ay);
                    intersections.push_back(ix);
                }

                std::sort(intersections.begin(), intersections.end());

                for (usize i = 0; i + 1 < intersections.size(); i += 2) {
                    emit_clipped_scanline<M>(fb, intersections[i], intersections[i + 1], y, c);
                }
            }
        }

        template<BlendMode M>
        auto blit_impl(FrameBuffer &fb, const Texture &tex, Vec2i pos) noexcept -> void {
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
                    write_pixel<M>(
                        fb, px + sx, py + sy,
                        Color::from_argb(tex.get_pixel_argb_unchecked(sx, sy))
                    );
                }
            }
        }

        template<BlendMode M, SamplerFilter F, WrapMode W>
        auto blit_ex_impl(
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

            const std::array<Vec2f, 4> tex_corners = {
                Vec2f{0.f, 0.f}, Vec2f{tw, 0.f}, Vec2f{tw, th}, Vec2f{0.f, th}
            };

            f32 min_x = std::numeric_limits<f32>::max();
            f32 min_y = std::numeric_limits<f32>::max();
            f32 max_x = std::numeric_limits<f32>::lowest();
            f32 max_y = std::numeric_limits<f32>::lowest();
            for (const auto &uv: tex_corners) {
                const f32 dx = (uv.x() - origin.x()) * sx;
                const f32 dy = (uv.y() - origin.y()) * sy;
                const f32 rx = pos.x() + dx * cos_a - dy * sin_a;
                const f32 ry = pos.y() + dx * sin_a + dy * cos_a;
                min_x = std::min(min_x, rx);
                min_y = std::min(min_y, ry);
                max_x = std::max(max_x, rx);
                max_y = std::max(max_y, ry);
            }

            const i32 dst_x0 = std::max(0, static_cast<i32>(min_x));
            const i32 dst_y0 = std::max(0, static_cast<i32>(min_y));
            const i32 dst_x1 = std::min(fb.width() - 1, static_cast<i32>(max_x) + 1);
            const i32 dst_y1 = std::min(fb.height() - 1, static_cast<i32>(max_y) + 1);

            const f32 inv_sx = 1.f / sx;
            const f32 inv_sy = 1.f / sy;
            const f32 step_tx = cos_a * inv_sx;
            const f32 step_ty = -sin_a * inv_sy;
            const f32 base_x = static_cast<f32>(dst_x0);

            // narrow optimization: only when Nearest+Clamp (OOB = no write)
            constexpr bool can_narrow = (F == SamplerFilter::Nearest) && (W == WrapMode::Clamp);

            const f32 ftw = static_cast<f32>(tex.width());
            const f32 fth = static_cast<f32>(tex.height());

            const auto narrow = [&](f32 val0, f32 step, f32 dim, i32 &lo, i32 &hi) noexcept {
                if (step > 0.f) {
                    lo = std::max(lo, static_cast<i32>(std::ceil(base_x + (0.f - val0) / step)));
                    hi = std::min(hi, static_cast<i32>(std::ceil(base_x + (dim - val0) / step)) - 1);
                } else if (step < 0.f) {
                    lo = std::max(lo, static_cast<i32>(std::floor(base_x + (dim - val0) / step)) + 1);
                    hi = std::min(hi, static_cast<i32>(std::floor(base_x + (0.f - val0) / step)));
                } else if (val0 < 0.f || val0 >= dim) {
                    hi = lo - 1;
                }
            };

            for (i32 dy = dst_y0; dy <= dst_y1; ++dy) {
                const f32 rx0 = base_x - pos.x();
                const f32 ry = static_cast<f32>(dy) - pos.y();
                const f32 tx0 = (rx0 * cos_a + ry * sin_a) * inv_sx + origin.x();
                const f32 ty0 = (-rx0 * sin_a + ry * cos_a) * inv_sy + origin.y();

                i32 dx_lo = dst_x0;
                i32 dx_hi = dst_x1;
                if constexpr (can_narrow) {
                    narrow(tx0, step_tx, ftw, dx_lo, dx_hi);
                    narrow(ty0, step_ty, fth, dx_lo, dx_hi);
                    if (dx_lo > dx_hi) {
                        continue;
                    }
                }

                const f32 k = static_cast<f32>(dx_lo - dst_x0);
                f32 tx = tx0 + k * step_tx;
                f32 ty = ty0 + k * step_ty;

                if constexpr (can_narrow) {
                    // fast path: direct fetch + in_bounds safety net (FP edge slack)
                    for (i32 dx = dx_lo; dx <= dx_hi; ++dx) {
                        const i32 itx = static_cast<i32>(std::floor(tx));
                        const i32 ity = static_cast<i32>(std::floor(ty));
                        if (tex.in_bounds(itx, ity)) {
                            write_pixel<M>(fb, dx, dy,
                                           Color::from_argb(tex.get_pixel_argb_unchecked(itx, ity)));
                        }
                        tx += step_tx;
                        ty += step_ty;
                    }
                } else {
                    for (i32 dx = dx_lo; dx <= dx_hi; ++dx) {
                        write_pixel<M>(fb, dx, dy, sample<F, W>(tex, tx, ty));
                        tx += step_tx;
                        ty += step_ty;
                    }
                }
            }
        }
    } // anonymous namespace

    // -------- public API: runtime enum → compile-time dispatch --------

    auto draw_line(FrameBuffer &fb, Vec2i a, Vec2i b, Color c, BlendMode mode) noexcept -> void {
        dispatch_blend(mode, [&]<BlendMode M>(std::integral_constant<BlendMode, M>) {
            draw_line_impl<M>(fb, a, b, c);
        });
    }

    auto draw_line_ex(FrameBuffer &fb, Vec2i a, Vec2i b, f32 thickness, Color c, BlendMode mode) noexcept -> void {
        dispatch_blend(mode, [&]<BlendMode M>(std::integral_constant<BlendMode, M>) {
            draw_line_ex_impl<M>(fb, a, b, thickness, c);
        });
    }

    auto draw_rect(FrameBuffer &fb, Vec2i a, Vec2i b, Color c, BlendMode mode) noexcept -> void {
        dispatch_blend(mode, [&]<BlendMode M>(std::integral_constant<BlendMode, M>) {
            draw_rect_impl<M>(fb, a, b, c);
        });
    }

    auto fill_rect(FrameBuffer &fb, Vec2i a, Vec2i b, Color c, BlendMode mode) noexcept -> void {
        dispatch_blend(mode, [&]<BlendMode M>(std::integral_constant<BlendMode, M>) {
            fill_rect_impl<M>(fb, a, b, c);
        });
    }

    auto draw_circle(FrameBuffer &fb, Vec2i center, i32 r, Color c, BlendMode mode) noexcept -> void {
        dispatch_blend(mode, [&]<BlendMode M>(std::integral_constant<BlendMode, M>) {
            draw_circle_impl<M>(fb, center, r, c);
        });
    }

    auto fill_circle(FrameBuffer &fb, Vec2i center, i32 r, Color c, BlendMode mode) noexcept -> void {
        dispatch_blend(mode, [&]<BlendMode M>(std::integral_constant<BlendMode, M>) {
            fill_circle_impl<M>(fb, center, r, c);
        });
    }

    auto draw_ellipse(FrameBuffer &fb, Vec2i center, i32 rx, i32 ry, Color c, BlendMode mode) noexcept -> void {
        dispatch_blend(mode, [&]<BlendMode M>(std::integral_constant<BlendMode, M>) {
            draw_ellipse_impl<M>(fb, center, rx, ry, c);
        });
    }

    auto fill_ellipse(FrameBuffer &fb, Vec2i center, i32 rx, i32 ry, Color c, BlendMode mode) noexcept -> void {
        dispatch_blend(mode, [&]<BlendMode M>(std::integral_constant<BlendMode, M>) {
            fill_ellipse_impl<M>(fb, center, rx, ry, c);
        });
    }

    auto draw_triangle(FrameBuffer &fb, Vec2i a, Vec2i b, Vec2i c, Color col, BlendMode mode) noexcept -> void {
        dispatch_blend(mode, [&]<BlendMode M>(std::integral_constant<BlendMode, M>) {
            draw_triangle_impl<M>(fb, a, b, c, col);
        });
    }

    auto fill_triangle(FrameBuffer &fb, Vec2i a, Vec2i b, Vec2i c, Color col, BlendMode mode) noexcept -> void {
        dispatch_blend(mode, [&]<BlendMode M>(std::integral_constant<BlendMode, M>) {
            fill_triangle_impl<M>(fb, a, b, c, col);
        });
    }

    auto draw_polygon(FrameBuffer &fb, std::span<const Vec2i> points, Color c, BlendMode mode) noexcept -> void {
        dispatch_blend(mode, [&]<BlendMode M>(std::integral_constant<BlendMode, M>) {
            draw_polygon_impl<M>(fb, points, c);
        });
    }

    auto fill_polygon(FrameBuffer &fb, std::span<const Vec2i> points, Color c, BlendMode mode) noexcept -> void {
        dispatch_blend(mode, [&]<BlendMode M>(std::integral_constant<BlendMode, M>) {
            fill_polygon_impl<M>(fb, points, c);
        });
    }

    auto blit(FrameBuffer &fb, const Texture &tex, Vec2i pos, BlendMode mode) noexcept -> void {
        dispatch_blend(mode, [&]<BlendMode M>(std::integral_constant<BlendMode, M>) {
            blit_impl<M>(fb, tex, pos);
        });
    }

    auto blit_ex(
        FrameBuffer &fb, const Texture &tex, Vec2f pos, Vec2f origin, Vec2f scale, f32 angle,
        BlendMode mode, SamplerFilter filter, WrapMode wrap
    ) noexcept -> void {
        // 3 × 2 × 3 = 18 compile-time instantiations via nested dispatchers
        dispatch_blend(mode, [&]<BlendMode M>(std::integral_constant<BlendMode, M>) {
            dispatch_filter(filter, [&]<SamplerFilter F>(std::integral_constant<SamplerFilter, F>) {
                dispatch_wrap(wrap, [&]<WrapMode W>(std::integral_constant<WrapMode, W>) {
                    blit_ex_impl<M, F, W>(fb, tex, pos, origin, scale, angle);
                });
            });
        });
    }
}
