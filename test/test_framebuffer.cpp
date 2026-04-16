#include <gtest/gtest.h>

#include "sr/gfx/framebuffer.h"

using namespace sr;

namespace {
    auto make_fb(i32 w = 16, i32 h = 16) -> FrameBuffer {
        auto r = FrameBuffer::create(w, h);
        EXPECT_TRUE(r.has_value());
        return std::move(*r);
    }
}

TEST(FrameBuffer, CreateRejectsNonPositiveDims) {
    EXPECT_FALSE(FrameBuffer::create(0, 10).has_value());
    EXPECT_FALSE(FrameBuffer::create(10, 0).has_value());
    EXPECT_FALSE(FrameBuffer::create(-1, 10).has_value());
    EXPECT_FALSE(FrameBuffer::create(10, -1).has_value());
}

TEST(FrameBuffer, CreateZeroesBuffer) {
    auto fb = make_fb(4, 4);
    for (i32 y = 0; y < fb.height(); ++y) {
        for (i32 x = 0; x < fb.width(); ++x) {
            EXPECT_EQ(fb.get_pixel(x, y), colors::transparent);
        }
    }
}

TEST(FrameBuffer, Dimensions) {
    auto fb = make_fb(7, 11);
    EXPECT_EQ(fb.width(), 7);
    EXPECT_EQ(fb.height(), 11);
    EXPECT_EQ(fb.stride(), 7);
}

TEST(FrameBuffer, InBounds) {
    auto fb = make_fb(4, 4);
    EXPECT_TRUE(fb.in_bounds(0, 0));
    EXPECT_TRUE(fb.in_bounds(3, 3));
    EXPECT_FALSE(fb.in_bounds(4, 3));
    EXPECT_FALSE(fb.in_bounds(3, 4));
    EXPECT_FALSE(fb.in_bounds(-1, 0));
    EXPECT_FALSE(fb.in_bounds(0, -1));
}

TEST(FrameBuffer, GetPixelOOBIsTransparent) {
    auto fb = make_fb(4, 4);
    EXPECT_EQ(fb.get_pixel(-1, 0), colors::transparent);
    EXPECT_EQ(fb.get_pixel(4, 0), colors::transparent);
    EXPECT_EQ(fb.get_pixel(0, 100), colors::transparent);
}

TEST(FrameBuffer, SetPixelOOBIsSilentNoOp) {
    auto fb = make_fb(4, 4);
    fb.set_pixel(-1, 0, colors::red);
    fb.set_pixel(4, 0, colors::red);
    fb.set_pixel(0, 100, colors::red);
    // every in-bounds pixel still default
    for (i32 y = 0; y < 4; ++y) {
        for (i32 x = 0; x < 4; ++x) {
            EXPECT_EQ(fb.get_pixel(x, y), colors::transparent);
        }
    }
}

TEST(FrameBuffer, SetGetPixelRoundtripColor) {
    auto fb = make_fb();
    fb.set_pixel(3, 4, colors::red);
    EXPECT_EQ(fb.get_pixel(3, 4), colors::red);
}

TEST(FrameBuffer, SetGetPixelRoundtripPixel) {
    auto fb = make_fb();
    const Pixel p = colors::green.to_argb();
    fb.set_pixel(1, 2, p);
    EXPECT_EQ(fb.get_pixel(1, 2), colors::green);
}

TEST(FrameBuffer, UncheckedSetGet) {
    auto fb = make_fb();
    fb.set_pixel_unchecked(0, 0, colors::blue);
    EXPECT_EQ(fb.get_pixel_unchecked(0, 0), colors::blue);
}

TEST(FrameBuffer, ClearColor) {
    auto fb = make_fb(3, 3);
    fb.clear(colors::red);
    for (i32 y = 0; y < 3; ++y) {
        for (i32 x = 0; x < 3; ++x) {
            EXPECT_EQ(fb.get_pixel(x, y), colors::red);
        }
    }
}

TEST(FrameBuffer, ClearPixel) {
    auto fb = make_fb(2, 2);
    fb.clear(colors::white.to_argb());
    EXPECT_EQ(fb.get_pixel(0, 0), colors::white);
    EXPECT_EQ(fb.get_pixel(1, 1), colors::white);
}

TEST(FrameBuffer, FillHorLineFullRow) {
    auto fb = make_fb(8, 4);
    fb.fill_hor_line(0, 7, 2, colors::red.to_argb());
    for (i32 x = 0; x < 8; ++x) {
        EXPECT_EQ(fb.get_pixel(x, 2), colors::red);
    }
    // adjacent rows untouched
    EXPECT_EQ(fb.get_pixel(0, 1), colors::transparent);
    EXPECT_EQ(fb.get_pixel(0, 3), colors::transparent);
}

TEST(FrameBuffer, FillHorLineSwappedXIsNormalized) {
    auto fb = make_fb(8, 4);
    fb.fill_hor_line(5, 2, 0, colors::red.to_argb());
    for (i32 x = 2; x <= 5; ++x) {
        EXPECT_EQ(fb.get_pixel(x, 0), colors::red);
    }
    EXPECT_EQ(fb.get_pixel(1, 0), colors::transparent);
    EXPECT_EQ(fb.get_pixel(6, 0), colors::transparent);
}

TEST(FrameBuffer, FillHorLineClipsToBounds) {
    auto fb = make_fb(8, 4);
    fb.fill_hor_line(-100, 100, 1, colors::red.to_argb());
    for (i32 x = 0; x < 8; ++x) {
        EXPECT_EQ(fb.get_pixel(x, 1), colors::red);
    }
}

TEST(FrameBuffer, FillHorLineOOBYIsNoOp) {
    auto fb = make_fb(8, 4);
    fb.fill_hor_line(0, 7, -1, colors::red.to_argb());
    fb.fill_hor_line(0, 7, 100, colors::red.to_argb());
    for (i32 y = 0; y < 4; ++y) {
        for (i32 x = 0; x < 8; ++x) {
            EXPECT_EQ(fb.get_pixel(x, y), colors::transparent);
        }
    }
}

TEST(FrameBuffer, FillHorLineColorTransparentIsNoOp) {
    auto fb = make_fb(4, 1);
    fb.clear(colors::red);
    fb.fill_hor_line(0, 3, 0, Color{0, 0, 0, 0});
    for (i32 x = 0; x < 4; ++x) {
        EXPECT_EQ(fb.get_pixel(x, 0), colors::red);
    }
}

TEST(FrameBuffer, FillHorLineColorBlendsWhenAlphaPartial) {
    auto fb = make_fb(4, 1);
    fb.clear(colors::black);
    fb.fill_hor_line(0, 3, 0, Color{255, 0, 0, 128}); // half-alpha red over opaque black
    for (i32 x = 0; x < 4; ++x) {
        const auto px = fb.get_pixel(x, 0);
        EXPECT_EQ(px.r, 128);
        EXPECT_EQ(px.g, 0);
        EXPECT_EQ(px.b, 0);
        EXPECT_EQ(px.a, 255);
    }
}

TEST(FrameBuffer, ResizeRejectsNonPositive) {
    auto fb = make_fb(4, 4);
    EXPECT_FALSE(fb.resize(0, 4).has_value());
    EXPECT_FALSE(fb.resize(4, -1).has_value());
    // state preserved on failure
    EXPECT_EQ(fb.width(), 4);
    EXPECT_EQ(fb.height(), 4);
}

TEST(FrameBuffer, ResizeChangesDimsAndZeroes) {
    auto fb = make_fb(4, 4);
    fb.clear(colors::red);
    ASSERT_TRUE(fb.resize(2, 6).has_value());
    EXPECT_EQ(fb.width(), 2);
    EXPECT_EQ(fb.height(), 6);
    for (i32 y = 0; y < 6; ++y) {
        for (i32 x = 0; x < 2; ++x) {
            EXPECT_EQ(fb.get_pixel(x, y), colors::transparent);
        }
    }
}
