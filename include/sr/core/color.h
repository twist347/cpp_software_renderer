#pragma once

#include "sr/core/types.h"

namespace sr {
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

        // Porter-Duff "source over"
        [[nodiscard]] auto blend_over(Color dst) const noexcept -> Color;

        [[nodiscard]] static auto lerp(Color a, Color b, f32 t) noexcept -> Color;

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
