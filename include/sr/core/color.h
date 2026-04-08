#pragma once

#include "sr/core/types.h"

namespace sr {
    // 32-bit pixel: 0xAARRGGBB. Storage type for FrameBuffer and Texture buffers.
    using Pixel = u32;

    struct Color {
        u8 r{};
        u8 g{};
        u8 b{};
        u8 a{255};

        constexpr Color() noexcept = default;

        constexpr Color(u8 r, u8 g, u8 b, u8 a = 255) noexcept
            : r{r}, g{g}, b{b}, a{a} {
        }

        [[nodiscard]] constexpr auto to_argb() const noexcept -> u32 {
            return static_cast<u32>(a) << 24
                   | static_cast<u32>(r) << 16
                   | static_cast<u32>(g) << 8
                   | static_cast<u32>(b);
        }

        [[nodiscard]] static constexpr auto from_argb(u32 c) noexcept -> Color {
            return {
                static_cast<u8>(c >> 16),
                static_cast<u8>(c >> 8),
                static_cast<u8>(c),
                static_cast<u8>(c >> 24)
            };
        }

        // Porter-Duff "source over"; integer-only — avoids float conversions in hot path
        [[nodiscard]] constexpr auto blend_over(Color dst) const noexcept -> Color {
            const u32 sa = a;
            const u32 inv = 255 - sa;
            return {
                static_cast<u8>((r * sa + dst.r * inv + 127) / 255),
                static_cast<u8>((g * sa + dst.g * inv + 127) / 255),
                static_cast<u8>((b * sa + dst.b * inv + 127) / 255),
                static_cast<u8>((a * 255 + dst.a * inv + 127) / 255)
            };
        }

        [[nodiscard]] static constexpr auto lerp(Color a, Color b, f32 t) noexcept -> Color {
            const f32 inv = 1.f - t;
            return {
                static_cast<u8>(static_cast<f32>(a.r) * inv + static_cast<f32>(b.r) * t + 0.5f),
                static_cast<u8>(static_cast<f32>(a.g) * inv + static_cast<f32>(b.g) * t + 0.5f),
                static_cast<u8>(static_cast<f32>(a.b) * inv + static_cast<f32>(b.b) * t + 0.5f),
                static_cast<u8>(static_cast<f32>(a.a) * inv + static_cast<f32>(b.a) * t + 0.5f)
            };
        }

        constexpr bool operator==(const Color &) const noexcept = default;
    };

    namespace colors {
        inline constexpr auto black = Color{0, 0, 0};
        inline constexpr auto white = Color{255, 255, 255};
        inline constexpr auto red = Color{255, 0, 0};
        inline constexpr auto green = Color{0, 255, 0};
        inline constexpr auto blue = Color{0, 0, 255};
    }
}
