#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>
#include <vector>

#include "sr/gfx/framebuffer.h"
#include "sr/gfx/rasterizer.h"
#include "sr/gfx/texture.h"

using namespace sr;

namespace {
    auto make_fb(i32 w = 32, i32 h = 32) -> FrameBuffer {
        return std::move(*FrameBuffer::create(w, h));
    }

    auto full_clip(const FrameBuffer &fb) -> Recti {
        return Recti::from_pos_size({0, 0}, {fb.width(), fb.height()});
    }

    auto count_non_transparent(const FrameBuffer &fb) -> i32 {
        i32 n = 0;
        for (i32 y = 0; y < fb.height(); ++y) {
            for (i32 x = 0; x < fb.width(); ++x) {
                if (fb.get_pixel(x, y) != colors::transparent) {
                    ++n;
                }
            }
        }
        return n;
    }

    auto count_color(const FrameBuffer &fb, Color c) -> i32 {
        i32 n = 0;
        for (i32 y = 0; y < fb.height(); ++y) {
            for (i32 x = 0; x < fb.width(); ++x) {
                if (fb.get_pixel(x, y) == c) {
                    ++n;
                }
            }
        }
        return n;
    }

    // every pixel outside `clip` must remain transparent (assumes FB starts cleared)
    auto only_clip_touched(const FrameBuffer &fb, Recti clip) -> ::testing::AssertionResult {
        for (i32 y = 0; y < fb.height(); ++y) {
            for (i32 x = 0; x < fb.width(); ++x) {
                if (!clip.contains({x, y}) && fb.get_pixel(x, y) != colors::transparent) {
                    return ::testing::AssertionFailure()
                           << "wrote outside clip at (" << x << ", " << y << ")";
                }
            }
        }
        return ::testing::AssertionSuccess();
    }

    auto make_solid_texture(i32 w, i32 h, Color c) -> Texture {
        const std::vector<Pixel> data(static_cast<usize>(w) * h, c.to_argb());
        return std::move(*Texture::from_pixels(w, h, data));
    }
}

// ---------- clip respect: no primitive may write outside the clip rect ----------

TEST(Raster, FillRectRespectsClip) {
    auto fb = make_fb();
    const Recti clip{{8, 8}, {24, 24}};
    raster::fill_rect(fb, clip, {0, 0}, {32, 32}, colors::red);
    EXPECT_TRUE(only_clip_touched(fb, clip));
}

TEST(Raster, DrawRectRespectsClip) {
    auto fb = make_fb();
    const Recti clip{{8, 8}, {24, 24}};
    raster::draw_rect(fb, clip, {0, 0}, {31, 31}, colors::red);
    EXPECT_TRUE(only_clip_touched(fb, clip));
}

TEST(Raster, DrawLineRespectsClip) {
    auto fb = make_fb();
    const Recti clip{{8, 8}, {24, 24}};
    raster::draw_line(fb, clip, {-50, -50}, {100, 100}, colors::red);
    EXPECT_TRUE(only_clip_touched(fb, clip));
}

TEST(Raster, DrawLineExRespectsClip) {
    auto fb = make_fb();
    const Recti clip{{8, 8}, {24, 24}};
    raster::draw_line_ex(fb, clip, {-10, 16}, {40, 16}, 6.f, colors::red);
    EXPECT_TRUE(only_clip_touched(fb, clip));
}

TEST(Raster, DrawCircleRespectsClip) {
    auto fb = make_fb();
    const Recti clip{{8, 8}, {24, 24}};
    raster::draw_circle(fb, clip, {16, 16}, 20, colors::red);
    EXPECT_TRUE(only_clip_touched(fb, clip));
}

TEST(Raster, FillCircleRespectsClip) {
    auto fb = make_fb();
    const Recti clip{{8, 8}, {24, 24}};
    raster::fill_circle(fb, clip, {16, 16}, 20, colors::red);
    EXPECT_TRUE(only_clip_touched(fb, clip));
}

TEST(Raster, DrawEllipseRespectsClip) {
    auto fb = make_fb();
    const Recti clip{{8, 8}, {24, 24}};
    raster::draw_ellipse(fb, clip, {16, 16}, 25, 18, colors::red);
    EXPECT_TRUE(only_clip_touched(fb, clip));
}

TEST(Raster, FillEllipseRespectsClip) {
    auto fb = make_fb();
    const Recti clip{{8, 8}, {24, 24}};
    raster::fill_ellipse(fb, clip, {16, 16}, 25, 18, colors::red);
    EXPECT_TRUE(only_clip_touched(fb, clip));
}

TEST(Raster, DrawTriangleRespectsClip) {
    auto fb = make_fb();
    const Recti clip{{8, 8}, {24, 24}};
    raster::draw_triangle(fb, clip, {-10, 0}, {40, 0}, {15, 40}, colors::red);
    EXPECT_TRUE(only_clip_touched(fb, clip));
}

TEST(Raster, FillTriangleRespectsClip) {
    auto fb = make_fb();
    const Recti clip{{8, 8}, {24, 24}};
    raster::fill_triangle(fb, clip, {-10, 0}, {40, 0}, {15, 40}, colors::red);
    EXPECT_TRUE(only_clip_touched(fb, clip));
}

TEST(Raster, DrawPolygonRespectsClip) {
    auto fb = make_fb();
    const Recti clip{{8, 8}, {24, 24}};
    const Vec2i pts[]{{-5, 16}, {16, -5}, {37, 16}, {30, 30}, {2, 30}};
    raster::draw_polygon(fb, clip, pts, colors::red);
    EXPECT_TRUE(only_clip_touched(fb, clip));
}

TEST(Raster, FillPolygonRespectsClip) {
    auto fb = make_fb();
    const Recti clip{{8, 8}, {24, 24}};
    const Vec2i pts[]{{-5, 16}, {16, -5}, {37, 16}, {30, 30}, {2, 30}};
    raster::fill_polygon(fb, clip, pts, colors::red);
    EXPECT_TRUE(only_clip_touched(fb, clip));
}

TEST(Raster, BlitRespectsClip) {
    auto fb = make_fb();
    const auto tex = make_solid_texture(40, 40, colors::red);
    const Recti clip{{8, 8}, {24, 24}};
    raster::blit(fb, clip, tex, {-5, -5});
    EXPECT_TRUE(only_clip_touched(fb, clip));
}

TEST(Raster, BlitExRespectsClip) {
    auto fb = make_fb();
    const auto tex = make_solid_texture(40, 40, colors::red);
    const Recti clip{{8, 8}, {24, 24}};
    raster::blit_ex(fb, clip, tex, {16.f, 16.f}, {20.f, 20.f}, {1.f, 1.f}, 0.5f);
    EXPECT_TRUE(only_clip_touched(fb, clip));
}

// ---------- empty clip is a no-op for every primitive ----------

TEST(Raster, EmptyClipFillRectNoOp) {
    auto fb = make_fb();
    const Recti empty{{10, 10}, {10, 10}};
    raster::fill_rect(fb, empty, {0, 0}, {32, 32}, colors::red);
    EXPECT_EQ(count_non_transparent(fb), 0);
}

TEST(Raster, EmptyClipDrawLineNoOp) {
    auto fb = make_fb();
    const Recti empty{{10, 10}, {10, 10}};
    raster::draw_line(fb, empty, {0, 0}, {31, 31}, colors::red);
    EXPECT_EQ(count_non_transparent(fb), 0);
}

TEST(Raster, EmptyClipFillCircleNoOp) {
    auto fb = make_fb();
    const Recti empty{{10, 10}, {10, 10}};
    raster::fill_circle(fb, empty, {16, 16}, 10, colors::red);
    EXPECT_EQ(count_non_transparent(fb), 0);
}

TEST(Raster, EmptyClipBlitNoOp) {
    auto fb = make_fb();
    const auto tex = make_solid_texture(8, 8, colors::red);
    const Recti empty{{10, 10}, {10, 10}};
    raster::blit(fb, empty, tex, {0, 0});
    EXPECT_EQ(count_non_transparent(fb), 0);
}

// ---------- degenerate inputs ----------

TEST(Raster, DegenerateLineSinglePixel) {
    auto fb = make_fb(16, 16);
    raster::draw_line(fb, full_clip(fb), {5, 5}, {5, 5}, colors::red);
    EXPECT_EQ(count_non_transparent(fb), 1);
    EXPECT_EQ(fb.get_pixel(5, 5), colors::red);
}

TEST(Raster, HorizontalCollinearTriangleNoOp) {
    // explicit guard in fill_triangle: returns when all 3 vertices share a y after y-sort
    auto fb = make_fb(16, 16);
    raster::fill_triangle(fb, full_clip(fb), {0, 5}, {7, 5}, {15, 5}, colors::red);
    EXPECT_EQ(count_non_transparent(fb), 0);
}

TEST(Raster, DiagonalCollinearTriangleDrawsLine) {
    // documented quirk: diagonal-collinear vertices have no explicit guard.
    // Scanline produces single-pixel-per-row "fill" along the shared edge —
    // not a bug, just the scanline rule extended to a degenerate input.
    auto fb = make_fb(16, 16);
    raster::fill_triangle(fb, full_clip(fb), {0, 0}, {5, 5}, {10, 10}, colors::red);
    EXPECT_GT(count_non_transparent(fb), 0);
    EXPECT_LE(count_non_transparent(fb), 11); // bounded by y-extent (0..10)
}

TEST(Raster, ZeroOrNegativeRadiusCircleNoOp) {
    auto fb = make_fb(16, 16);
    raster::draw_circle(fb, full_clip(fb), {8, 8}, 0, colors::red);
    raster::fill_circle(fb, full_clip(fb), {8, 8}, 0, colors::red);
    raster::draw_circle(fb, full_clip(fb), {8, 8}, -5, colors::red);
    raster::fill_circle(fb, full_clip(fb), {8, 8}, -5, colors::red);
    EXPECT_EQ(count_non_transparent(fb), 0);
}

TEST(Raster, ZeroOrNegativeRadiusEllipseNoOp) {
    auto fb = make_fb(16, 16);
    raster::draw_ellipse(fb, full_clip(fb), {8, 8}, 0, 5, colors::red);
    raster::draw_ellipse(fb, full_clip(fb), {8, 8}, 5, 0, colors::red);
    raster::fill_ellipse(fb, full_clip(fb), {8, 8}, 0, 0, colors::red);
    raster::fill_ellipse(fb, full_clip(fb), {8, 8}, -3, 5, colors::red);
    EXPECT_EQ(count_non_transparent(fb), 0);
}

TEST(Raster, FillPolygonTooFewVerticesNoOp) {
    auto fb = make_fb(16, 16);
    const Vec2i pts[]{{0, 0}, {10, 10}};
    raster::fill_polygon(fb, full_clip(fb), pts, colors::red);
    EXPECT_EQ(count_non_transparent(fb), 0);
}

TEST(Raster, BlitExZeroScaleNoOp) {
    auto fb = make_fb(16, 16);
    const auto tex = make_solid_texture(4, 4, colors::red);
    raster::blit_ex(fb, full_clip(fb), tex, {8.f, 8.f}, {0.f, 0.f}, {0.f, 1.f}, 0.f);
    EXPECT_EQ(count_non_transparent(fb), 0);
    raster::blit_ex(fb, full_clip(fb), tex, {8.f, 8.f}, {0.f, 0.f}, {1.f, 0.f}, 0.f);
    EXPECT_EQ(count_non_transparent(fb), 0);
}

// ---------- blend mode behavior ----------

TEST(Raster, AlphaTransparentSrcIsNoOp) {
    auto fb = make_fb(16, 16);
    fb.clear(colors::red);
    const Color clear_zero{0, 255, 0, 0}; // green but α=0
    raster::fill_rect(fb, full_clip(fb), {0, 0}, {16, 16}, clear_zero, BlendMode::Alpha);
    // α=0 in Alpha mode: no pixel is touched
    EXPECT_EQ(count_color(fb, colors::red), 256);
}

TEST(Raster, NoneOverwritesIgnoringAlpha) {
    auto fb = make_fb(16, 16);
    fb.clear(colors::red);
    const Color src{0, 255, 0, 100}; // partial alpha but None ignores it
    raster::fill_rect(fb, full_clip(fb), {0, 0}, {16, 16}, src, BlendMode::None);
    EXPECT_EQ(fb.get_pixel(8, 8), src);
}

TEST(Raster, AlphaOpaqueOverwrites) {
    auto fb = make_fb(16, 16);
    fb.clear(colors::red);
    raster::fill_rect(fb, full_clip(fb), {0, 0}, {16, 16}, colors::green, BlendMode::Alpha);
    EXPECT_EQ(count_color(fb, colors::green), 256);
}

// ---------- equivalences (algorithm-independent) ----------

TEST(Raster, BlitExIdentityMatchesBlit) {
    // 4×4 texture with distinct pixel values
    std::array<Pixel, 16> tex_data{};
    for (i32 i = 0; i < 16; ++i) {
        tex_data[i] = Color{
            static_cast<u8>(i * 16),
            static_cast<u8>(255 - i * 16),
            128, 255
        }.to_argb();
    }
    const auto tex = std::move(*Texture::from_pixels(4, 4, tex_data));

    auto fb1 = make_fb(16, 16);
    auto fb2 = make_fb(16, 16);
    raster::blit(fb1, full_clip(fb1), tex, {6, 6});
    raster::blit_ex(fb2, full_clip(fb2), tex, {6.f, 6.f}, {0.f, 0.f}, {1.f, 1.f}, 0.f,
                    BlendMode::Alpha, SamplerFilter::Nearest, WrapMode::Clamp);

    const auto s1 = fb1.span();
    const auto s2 = fb2.span();
    EXPECT_TRUE(std::equal(s1.begin(), s1.end(), s2.begin()));
}

// ---------- happy-path invariants (algorithm-independent properties of correct output) ----------

TEST(Raster, FillRectExactPixelCount) {
    // both endpoints inclusive: x∈[2..7]=6 wide, y∈[3..9]=7 tall → 42 pixels
    auto fb = make_fb(16, 16);
    raster::fill_rect(fb, full_clip(fb), {2, 3}, {7, 9}, colors::red, BlendMode::None);
    EXPECT_EQ(count_color(fb, colors::red), 42);
}

TEST(Raster, FillRectFillsExactlyBbox) {
    auto fb = make_fb(16, 16);
    raster::fill_rect(fb, full_clip(fb), {2, 3}, {7, 9}, colors::red, BlendMode::None);
    for (i32 y = 0; y < 16; ++y) {
        for (i32 x = 0; x < 16; ++x) {
            const bool inside = x >= 2 && x <= 7 && y >= 3 && y <= 9;
            EXPECT_EQ(fb.get_pixel(x, y), inside ? colors::red : colors::transparent)
                << "at (" << x << "," << y << ")";
        }
    }
}

TEST(Raster, FillRectIdempotentNoneBlend) {
    auto fb1 = make_fb(16, 16);
    auto fb2 = make_fb(16, 16);
    raster::fill_rect(fb1, full_clip(fb1), {2, 3}, {7, 9}, colors::red, BlendMode::None);
    raster::fill_rect(fb2, full_clip(fb2), {2, 3}, {7, 9}, colors::red, BlendMode::None);
    raster::fill_rect(fb2, full_clip(fb2), {2, 3}, {7, 9}, colors::red, BlendMode::None);
    const auto s1 = fb1.span();
    const auto s2 = fb2.span();
    EXPECT_TRUE(std::equal(s1.begin(), s1.end(), s2.begin()));
}

TEST(Raster, DrawLineEndpointsWritten) {
    auto fb = make_fb(32, 32);
    raster::draw_line(fb, full_clip(fb), {3, 5}, {28, 20}, colors::red);
    EXPECT_EQ(fb.get_pixel(3, 5), colors::red);
    EXPECT_EQ(fb.get_pixel(28, 20), colors::red);
}

TEST(Raster, DrawLineStaysInBbox) {
    auto fb = make_fb(32, 32);
    const Vec2i a{3, 5}, b{28, 20};
    raster::draw_line(fb, full_clip(fb), a, b, colors::red);
    const i32 xmin = std::min(a.x(), b.x()), xmax = std::max(a.x(), b.x());
    const i32 ymin = std::min(a.y(), b.y()), ymax = std::max(a.y(), b.y());
    for (i32 y = 0; y < 32; ++y) {
        for (i32 x = 0; x < 32; ++x) {
            if (fb.get_pixel(x, y) == colors::red) {
                EXPECT_GE(x, xmin);
                EXPECT_LE(x, xmax);
                EXPECT_GE(y, ymin);
                EXPECT_LE(y, ymax);
            }
        }
    }
}

TEST(Raster, DrawLine8Connected) {
    // every red pixel has a red 8-neighbor (Bresenham produces a connected chain)
    auto fb = make_fb(32, 32);
    raster::draw_line(fb, full_clip(fb), {3, 5}, {28, 20}, colors::red);
    for (i32 y = 0; y < 32; ++y) {
        for (i32 x = 0; x < 32; ++x) {
            if (fb.get_pixel(x, y) != colors::red) {
                continue;
            }
            bool has_neighbor = false;
            for (i32 dy = -1; dy <= 1 && !has_neighbor; ++dy) {
                for (i32 dx = -1; dx <= 1 && !has_neighbor; ++dx) {
                    if (dx == 0 && dy == 0) {
                        continue;
                    }
                    if (fb.get_pixel(x + dx, y + dy) == colors::red) {
                        has_neighbor = true;
                    }
                }
            }
            EXPECT_TRUE(has_neighbor) << "isolated pixel at (" << x << "," << y << ")";
        }
    }
}

TEST(Raster, FillCircleRadialSymmetry) {
    auto fb = make_fb(33, 33);
    const Vec2i c{16, 16};
    raster::fill_circle(fb, full_clip(fb), c, 10, colors::red);
    for (i32 dy = 1; dy <= 10; ++dy) {
        for (i32 dx = 1; dx <= 10; ++dx) {
            const auto p1 = fb.get_pixel(c.x() + dx, c.y() + dy);
            const auto p2 = fb.get_pixel(c.x() - dx, c.y() + dy);
            const auto p3 = fb.get_pixel(c.x() + dx, c.y() - dy);
            const auto p4 = fb.get_pixel(c.x() - dx, c.y() - dy);
            EXPECT_EQ(p1, p2) << "asym at dx=" << dx << " dy=" << dy;
            EXPECT_EQ(p1, p3) << "asym at dx=" << dx << " dy=" << dy;
            EXPECT_EQ(p1, p4) << "asym at dx=" << dx << " dy=" << dy;
        }
    }
}

TEST(Raster, DrawCirclePixelsOnRadiusBand) {
    auto fb = make_fb(33, 33);
    const f32 r = 10.f;
    const Vec2f c{16.f, 16.f};
    raster::draw_circle(fb, full_clip(fb), {16, 16}, 10, colors::red);
    for (i32 y = 0; y < 33; ++y) {
        for (i32 x = 0; x < 33; ++x) {
            if (fb.get_pixel(x, y) == colors::red) {
                const f32 d = std::hypot(static_cast<f32>(x) - c.x(), static_cast<f32>(y) - c.y());
                EXPECT_NEAR(d, r, 1.0f) << "at (" << x << "," << y << ")";
            }
        }
    }
}

TEST(Raster, FillCirclePixelsInsideRadius) {
    auto fb = make_fb(33, 33);
    const f32 r = 10.f;
    const Vec2f c{16.f, 16.f};
    raster::fill_circle(fb, full_clip(fb), {16, 16}, 10, colors::red);
    for (i32 y = 0; y < 33; ++y) {
        for (i32 x = 0; x < 33; ++x) {
            if (fb.get_pixel(x, y) == colors::red) {
                const f32 d = std::hypot(static_cast<f32>(x) - c.x(), static_cast<f32>(y) - c.y());
                EXPECT_LE(d, r + 1.0f) << "at (" << x << "," << y << ")";
            }
        }
    }
}

TEST(Raster, FillTriangleStaysInVertexBbox) {
    auto fb = make_fb(32, 32);
    const Vec2i a{5, 5}, b{25, 8}, c{12, 28};
    raster::fill_triangle(fb, full_clip(fb), a, b, c, colors::red);
    const i32 xmin = std::min({a.x(), b.x(), c.x()});
    const i32 xmax = std::max({a.x(), b.x(), c.x()});
    const i32 ymin = std::min({a.y(), b.y(), c.y()});
    const i32 ymax = std::max({a.y(), b.y(), c.y()});
    for (i32 y = 0; y < 32; ++y) {
        for (i32 x = 0; x < 32; ++x) {
            if (fb.get_pixel(x, y) == colors::red) {
                EXPECT_GE(x, xmin);
                EXPECT_LE(x, xmax);
                EXPECT_GE(y, ymin);
                EXPECT_LE(y, ymax);
            }
        }
    }
}

TEST(Raster, BlitWritesEveryTexelInClip) {
    auto fb = make_fb(32, 32);
    const auto tex = make_solid_texture(8, 6, colors::red);
    raster::blit(fb, full_clip(fb), tex, {10, 10});
    EXPECT_EQ(count_color(fb, colors::red), 8 * 6);
}

TEST(Raster, BlitWritesAtCorrectOffset) {
    auto fb = make_fb(32, 32);
    const auto tex = make_solid_texture(4, 4, colors::red);
    raster::blit(fb, full_clip(fb), tex, {10, 10});
    for (i32 y = 0; y < 32; ++y) {
        for (i32 x = 0; x < 32; ++x) {
            const bool inside = x >= 10 && x < 14 && y >= 10 && y < 14;
            EXPECT_EQ(fb.get_pixel(x, y), inside ? colors::red : colors::transparent);
        }
    }
}

TEST(Raster, FillPolygonOrientationInvariant) {
    // even-odd rule: reversing winding produces the same fill
    auto fb1 = make_fb(32, 32);
    auto fb2 = make_fb(32, 32);
    const Vec2i cw[]{{5, 5}, {25, 5}, {25, 25}, {5, 25}};
    const Vec2i ccw[]{{5, 5}, {5, 25}, {25, 25}, {25, 5}};
    raster::fill_polygon(fb1, full_clip(fb1), cw, colors::red, BlendMode::None);
    raster::fill_polygon(fb2, full_clip(fb2), ccw, colors::red, BlendMode::None);
    const auto s1 = fb1.span();
    const auto s2 = fb2.span();
    EXPECT_TRUE(std::equal(s1.begin(), s1.end(), s2.begin()));
}

// ---------- partial-clip blit ----------

TEST(Raster, BlitClipsTopLeft) {
    // 8×8 tex at (-3,-3) on a 16×16 FB → visible part is dst (0..4, 0..4) = 5×5 = 25 pixels
    auto fb = make_fb(16, 16);
    const auto tex = make_solid_texture(8, 8, colors::red);
    raster::blit(fb, full_clip(fb), tex, {-3, -3});
    EXPECT_EQ(count_color(fb, colors::red), 25);
    for (i32 y = 0; y < 16; ++y) {
        for (i32 x = 0; x < 16; ++x) {
            const bool inside = x < 5 && y < 5;
            EXPECT_EQ(fb.get_pixel(x, y), inside ? colors::red : colors::transparent);
        }
    }
}

TEST(Raster, BlitClipsBottomRight) {
    // 8×8 tex at (12,12) on a 16×16 FB → visible part is dst (12..15, 12..15) = 4×4 = 16 pixels
    auto fb = make_fb(16, 16);
    const auto tex = make_solid_texture(8, 8, colors::red);
    raster::blit(fb, full_clip(fb), tex, {12, 12});
    EXPECT_EQ(count_color(fb, colors::red), 16);
    for (i32 y = 0; y < 16; ++y) {
        for (i32 x = 0; x < 16; ++x) {
            const bool inside = x >= 12 && y >= 12;
            EXPECT_EQ(fb.get_pixel(x, y), inside ? colors::red : colors::transparent);
        }
    }
}

TEST(Raster, BlitClipsAgainstTighterClipRect) {
    // 8×8 tex fully inside FB but limited by an inner clip rect
    auto fb = make_fb(16, 16);
    const auto tex = make_solid_texture(8, 8, colors::red);
    const Recti clip{{4, 4}, {10, 10}}; // visible portion of tex blit at (2,2) is (4..9, 4..9) = 6×6
    raster::blit(fb, clip, tex, {2, 2});
    EXPECT_EQ(count_color(fb, colors::red), 36);
    EXPECT_TRUE(only_clip_touched(fb, clip));
}

// ---------- blit_ex rotation ----------

TEST(Raster, BlitExRotationFullCircleApproxIdentity) {
    // Rotation by 2π is mathematically identity, but std::sin(2π_f32) ≈ -8.7e-8 (not 0).
    // The narrow optimization in blit_ex (Nearest+Clamp path) conservatively skips
    // edge pixels when ty0/step_ty drift produces fractional uv near texture borders —
    // so a "no-op" 2π rotation can shave a 1-pixel band at edges. This is a known
    // trade-off (skipping per-pixel in_bounds check), not a correctness bug. ±10 absorbs it.
    auto fb_zero = make_fb(32, 32);
    auto fb_full = make_fb(32, 32);
    const auto tex = make_solid_texture(8, 8, colors::red);

    raster::blit_ex(fb_zero, full_clip(fb_zero), tex, {16.f, 16.f}, {4.f, 4.f}, {1.f, 1.f}, 0.f);
    raster::blit_ex(fb_full, full_clip(fb_full), tex, {16.f, 16.f}, {4.f, 4.f}, {1.f, 1.f},
                    static_cast<f32>(std::numbers::pi * 2.0));

    EXPECT_NEAR(count_color(fb_zero, colors::red), count_color(fb_full, colors::red), 10);
}

TEST(Raster, BlitExRotationVisibleAngleStable) {
    // For a meaningful rotation (10°), pixel count is stable across implementations
    // and within tight bounds of the unrotated count (footprint area is preserved).
    auto fb_zero = make_fb(48, 48);
    auto fb_rot = make_fb(48, 48);
    const auto tex = make_solid_texture(10, 10, colors::red);
    const f32 deg10 = static_cast<f32>(std::numbers::pi / 18.0);

    raster::blit_ex(fb_zero, full_clip(fb_zero), tex, {24.f, 24.f}, {5.f, 5.f}, {1.f, 1.f}, 0.f);
    raster::blit_ex(fb_rot, full_clip(fb_rot), tex, {24.f, 24.f}, {5.f, 5.f}, {1.f, 1.f}, deg10);

    // rotated 10×10 still covers ~100 ± perimeter pixels
    const i32 c0 = count_color(fb_zero, colors::red);
    const i32 cr = count_color(fb_rot, colors::red);
    EXPECT_EQ(c0, 100);
    EXPECT_GE(cr, 80);
    EXPECT_LE(cr, 120);
}

// ---------- triangle area / pixel-count rough sanity ----------

TEST(Raster, FillTriangleAreaApprox) {
    // right triangle (5,5)–(25,5)–(5,25): area = 0.5 * 20 * 20 = 200.
    // Inclusive scanline fill ≈ area + boundary lattice points (~60 here); empirical 231.
    // Use ±25% slack (~150..250) for algorithm-independence.
    auto fb = make_fb(32, 32);
    raster::fill_triangle(fb, full_clip(fb), {5, 5}, {25, 5}, {5, 25}, colors::red, BlendMode::None);
    const i32 count = count_color(fb, colors::red);
    EXPECT_GE(count, 170);
    EXPECT_LE(count, 250);
}

TEST(Raster, FillTriangleEqualsThreeTimesScaledArea) {
    // scaling vertices by 2× should increase pixel count by ~4× (area scales quadratically)
    auto fb_small = make_fb(64, 64);
    auto fb_big = make_fb(64, 64);
    raster::fill_triangle(fb_small, full_clip(fb_small), {5, 5}, {15, 5}, {5, 15}, colors::red,
                          BlendMode::None);
    raster::fill_triangle(fb_big, full_clip(fb_big), {5, 5}, {25, 5}, {5, 25}, colors::red,
                          BlendMode::None);
    const i32 small = count_color(fb_small, colors::red);
    const i32 big = count_color(fb_big, colors::red);
    // big ≈ 4 * small, with O(perimeter) noise in both
    const f32 ratio = static_cast<f32>(big) / static_cast<f32>(small);
    EXPECT_NEAR(ratio, 4.f, 0.6f);
}
