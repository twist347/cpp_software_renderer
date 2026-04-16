#include <gtest/gtest.h>

#include <array>

#include "sr/gfx/texture.h"

using namespace sr;

TEST(Texture, FromPixelsRejectsNonPositiveDims) {
    const std::array<Pixel, 4> px{};
    EXPECT_FALSE(Texture::from_pixels(0, 2, px).has_value());
    EXPECT_FALSE(Texture::from_pixels(2, 0, px).has_value());
    EXPECT_FALSE(Texture::from_pixels(-1, 2, px).has_value());
    EXPECT_FALSE(Texture::from_pixels(2, -1, px).has_value());
}

TEST(Texture, FromPixelsRejectsSizeMismatch) {
    const std::array<Pixel, 3> too_few{};
    EXPECT_FALSE(Texture::from_pixels(2, 2, too_few).has_value());

    const std::array<Pixel, 5> too_many{};
    EXPECT_FALSE(Texture::from_pixels(2, 2, too_many).has_value());
}

TEST(Texture, FromPixelsCopiesData) {
    const std::array<Pixel, 4> src = {
        colors::red.to_argb(),
        colors::green.to_argb(),
        colors::blue.to_argb(),
        colors::white.to_argb(),
    };
    auto r = Texture::from_pixels(2, 2, src);
    ASSERT_TRUE(r.has_value());
    const auto &tex = *r;

    EXPECT_EQ(tex.width(), 2);
    EXPECT_EQ(tex.height(), 2);
    EXPECT_EQ(tex.stride(), 2);

    EXPECT_EQ(tex.get_pixel(0, 0), colors::red);
    EXPECT_EQ(tex.get_pixel(1, 0), colors::green);
    EXPECT_EQ(tex.get_pixel(0, 1), colors::blue);
    EXPECT_EQ(tex.get_pixel(1, 1), colors::white);
}

TEST(Texture, GetPixelOOBIsTransparent) {
    const std::array<Pixel, 4> px = {
        colors::red.to_argb(), colors::red.to_argb(),
        colors::red.to_argb(), colors::red.to_argb(),
    };
    auto tex = std::move(*Texture::from_pixels(2, 2, px));
    EXPECT_EQ(tex.get_pixel(-1, 0), colors::transparent);
    EXPECT_EQ(tex.get_pixel(2, 0), colors::transparent);
    EXPECT_EQ(tex.get_pixel(0, 2), colors::transparent);
}

TEST(Texture, GetPixelArgbOOBIsZero) {
    const std::array<Pixel, 4> px = {
        colors::red.to_argb(), colors::red.to_argb(),
        colors::red.to_argb(), colors::red.to_argb(),
    };
    auto tex = std::move(*Texture::from_pixels(2, 2, px));
    EXPECT_EQ(tex.get_pixel_argb(-1, 0), 0u);
    EXPECT_EQ(tex.get_pixel_argb(2, 0), 0u);
    EXPECT_EQ(tex.get_pixel_argb(0, 2), 0u);
}

TEST(Texture, InBounds) {
    const std::array<Pixel, 6> px{};
    auto tex = std::move(*Texture::from_pixels(3, 2, px));
    EXPECT_TRUE(tex.in_bounds(0, 0));
    EXPECT_TRUE(tex.in_bounds(2, 1));
    EXPECT_FALSE(tex.in_bounds(3, 1));
    EXPECT_FALSE(tex.in_bounds(2, 2));
    EXPECT_FALSE(tex.in_bounds(-1, 0));
}

TEST(Texture, DataPointerMatchesGetPixel) {
    const std::array<Pixel, 4> src = {
        0x11223344u, 0x55667788u, 0x99aabbccu, 0xddeeff00u,
    };
    auto tex = std::move(*Texture::from_pixels(2, 2, src));
    const auto *data = tex.data();
    for (i32 y = 0; y < 2; ++y) {
        for (i32 x = 0; x < 2; ++x) {
            EXPECT_EQ(data[y * 2 + x], src[y * 2 + x]);
        }
    }
}
