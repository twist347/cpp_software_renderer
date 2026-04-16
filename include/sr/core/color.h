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

        [[nodiscard]] constexpr auto to_argb() const noexcept -> Pixel {
            return static_cast<Pixel>(a) << 24 |
                   static_cast<Pixel>(r) << 16 |
                   static_cast<Pixel>(g) << 8 |
                   static_cast<Pixel>(b);
        }

        [[nodiscard]] static constexpr auto from_argb(Pixel p) noexcept -> Color {
            return {
                static_cast<u8>(p >> 16),
                static_cast<u8>(p >> 8),
                static_cast<u8>(p),
                static_cast<u8>(p >> 24)
            };
        }

        // Porter-Duff "source over"; integer-only — avoids float conversions in hot path
        [[nodiscard]] constexpr auto blend_over(Color dst) const noexcept -> Color {
            const Pixel inv = 255 - a;
            return {
                static_cast<u8>((r * a + dst.r * inv + 127) / 255),
                static_cast<u8>((g * a + dst.g * inv + 127) / 255),
                static_cast<u8>((b * a + dst.b * inv + 127) / 255),
                static_cast<u8>((a * 255 + dst.a * inv + 127) / 255)
            };
        }

        // Q0.8 fixed-point lerp; `t_q8` in [0, 256]. Bit-exact for the bilinear sampler hot path.
        [[nodiscard]] static constexpr auto lerp(Color a, Color b, u32 t_q8) noexcept -> Color {
            const u32 inv = 256 - t_q8;
            return {
                static_cast<u8>((a.r * inv + b.r * t_q8 + 128) >> 8),
                static_cast<u8>((a.g * inv + b.g * t_q8 + 128) >> 8),
                static_cast<u8>((a.b * inv + b.b * t_q8 + 128) >> 8),
                static_cast<u8>((a.a * inv + b.a * t_q8 + 128) >> 8),
            };
        }

        constexpr bool operator==(const Color &) const noexcept = default;
    };

    namespace colors {
        inline constexpr auto transparent = Color{0, 0, 0, 0};
        inline constexpr auto black = Color{0, 0, 0};
        inline constexpr auto white = Color{255, 255, 255};
        inline constexpr auto gray = Color{128, 128, 128};
        inline constexpr auto red = Color{255, 0, 0};
        inline constexpr auto green = Color{0, 255, 0};
        inline constexpr auto blue = Color{0, 0, 255};
        inline constexpr auto yellow = Color{255, 255, 0};
        inline constexpr auto cyan = Color{0, 255, 255};
        inline constexpr auto magenta = Color{255, 0, 255};
    }
}
