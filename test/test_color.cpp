#include <gtest/gtest.h>

#include "sr/core/color.h"

using namespace sr;

TEST(Color, DefaultIsOpaqueBlack) {
    Color c{};
    EXPECT_EQ(c.r, 0);
    EXPECT_EQ(c.g, 0);
    EXPECT_EQ(c.b, 0);
    EXPECT_EQ(c.a, 255);
}

TEST(Color, ArgbRoundtrip) {
    const Color original{12, 34, 56, 78};
    EXPECT_EQ(Color::from_argb(original.to_argb()), original);
}

TEST(Color, ToArgbLayout) {
    const Color c{0x12, 0x34, 0x56, 0x78};
    EXPECT_EQ(c.to_argb(), 0x78123456u);
}

TEST(Color, FromArgbExtractsChannels) {
    const auto c = Color::from_argb(0x78123456u);
    EXPECT_EQ(c, Color(0x12, 0x34, 0x56, 0x78));
}

TEST(Color, BlendOverFullyOpaqueOverwrites) {
    const Color src{100, 50, 25, 255};
    const Color dst{200, 200, 200, 255};
    EXPECT_EQ(src.blend_over(dst), src);
}

TEST(Color, BlendOverFullyTransparentKeepsDst) {
    const Color src{255, 0, 0, 0};
    const Color dst{50, 60, 70, 128};
    EXPECT_EQ(src.blend_over(dst), dst);
}

TEST(Color, BlendOverHalfAlpha) {
    // src red @ a=128 over opaque black → ~half-bright red, opaque
    const Color src{255, 0, 0, 128};
    const Color dst{0, 0, 0, 255};
    const Color out = src.blend_over(dst);
    EXPECT_EQ(out.r, 128);
    EXPECT_EQ(out.g, 0);
    EXPECT_EQ(out.b, 0);
    EXPECT_EQ(out.a, 255);
}

TEST(Color, BlendOverPreservesOpaqueResult) {
    // any src over opaque dst stays opaque
    for (u32 a = 0; a <= 255; ++a) {
        const Color src{255, 255, 255, static_cast<u8>(a)};
        const Color dst{0, 0, 0, 255};
        EXPECT_EQ(src.blend_over(dst).a, 255) << "src.a=" << a;
    }
}

TEST(Color, LerpQ8Endpoints) {
    const Color a{10, 20, 30, 40};
    const Color b{200, 150, 100, 50};
    EXPECT_EQ(Color::lerp(a, b, 0u), a);
    EXPECT_EQ(Color::lerp(a, b, 256u), b);
}

TEST(Color, LerpQ8Midpoint) {
    const Color a{0, 0, 0, 0};
    const Color b{255, 255, 255, 255};
    const Color mid = Color::lerp(a, b, 128u);
    EXPECT_EQ(mid.r, 128);
    EXPECT_EQ(mid.g, 128);
    EXPECT_EQ(mid.b, 128);
    EXPECT_EQ(mid.a, 128);
}

TEST(Color, LerpQ8SymmetricUnderSwap) {
    const Color a{50, 100, 150, 200};
    const Color b{200, 150, 100, 50};
    // mid(a, b) at 128 ≈ mid(b, a) at 128 (off by ≤1 from rounding direction)
    const Color m1 = Color::lerp(a, b, 128u);
    const Color m2 = Color::lerp(b, a, 128u);
    EXPECT_LE(std::abs(m1.r - m2.r), 1);
    EXPECT_LE(std::abs(m1.g - m2.g), 1);
    EXPECT_LE(std::abs(m1.b - m2.b), 1);
    EXPECT_LE(std::abs(m1.a - m2.a), 1);
}

TEST(Color, NamedConstants) {
    EXPECT_EQ(colors::transparent, Color(0, 0, 0, 0));
    EXPECT_EQ(colors::black, Color(0, 0, 0));
    EXPECT_EQ(colors::white, Color(255, 255, 255));
    EXPECT_EQ(colors::red, Color(255, 0, 0));
    EXPECT_EQ(colors::green, Color(0, 255, 0));
    EXPECT_EQ(colors::blue, Color(0, 0, 255));
}
