#pragma once

#include <algorithm>

#include "sr/core/vec.h"

namespace sr {
    struct AABB2D {
        Vec2f min{};
        Vec2f max{};

        constexpr AABB2D() noexcept = default;

        constexpr AABB2D(const Vec2f &min, const Vec2f &max) noexcept : min{min}, max{max} {
        }

        static constexpr auto from_pos_size(Vec2f pos, Vec2f size) noexcept -> AABB2D {
            return {pos, pos + size};
        }

        [[nodiscard]] constexpr auto size() const noexcept -> Vec2f { return max - min; }
        [[nodiscard]] constexpr auto center() const noexcept -> Vec2f { return (min + max) * 0.5f; }

        [[nodiscard]] constexpr auto contains(Vec2f v) const noexcept -> bool {
            return v.x >= min.x && v.x <= max.x && v.y >= min.y && v.y <= max.y;
        }

        [[nodiscard]] constexpr auto intersects(const AABB2D &o) const noexcept -> bool {
            return min.x <= o.max.x && max.x >= o.min.x && min.y <= o.max.y && max.y >= o.min.y;
        }

        [[nodiscard]] constexpr auto clipped(const AABB2D &bounds) const noexcept -> AABB2D {
            return {
                {std::max(min.x, bounds.min.x), std::max(min.y, bounds.min.y)},
                {std::min(max.x, bounds.max.x), std::min(max.y, bounds.max.y)}
            };
        }

        [[nodiscard]] constexpr auto valid() const noexcept -> bool {
            return max.x >= min.x && max.y >= min.y;
        }
    };
}
