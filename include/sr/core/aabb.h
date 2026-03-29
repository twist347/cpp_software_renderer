#pragma once

#include "sr/core/vec.h"

namespace sr {
    struct AABB2D {
        Vec2f min{};
        Vec2f max{};

        constexpr AABB2D() noexcept = default;

        constexpr AABB2D(const Vec2f &min, const Vec2f &max) noexcept : min{min}, max{max} {
        }

        static constexpr AABB2D from_pos_size(Vec2f pos, Vec2f size) noexcept {
            return {pos, pos + size};
        }

        [[nodiscard]] constexpr Vec2f size() const noexcept { return max - min; }
        [[nodiscard]] constexpr Vec2f center() const noexcept { return (min + max) * 0.5f; }

        [[nodiscard]] constexpr bool contains(Vec2f v) const noexcept {
            return v.x >= min.x && v.x <= max.x && v.y >= min.y && v.y <= max.y;
        }

        [[nodiscard]] constexpr bool intersects(const AABB2D &o) const noexcept {
            return min.x <= o.max.x && max.x >= o.min.x && min.y <= o.max.y && max.y >= o.min.y;
        }

        [[nodiscard]] constexpr AABB2D clipped(const AABB2D &bounds) const noexcept {
            return {
                {std::max(min.x, bounds.min.x), std::max(min.y, bounds.min.y)},
                {std::min(max.x, bounds.max.x), std::min(max.y, bounds.max.y)}
            };
        }

        [[nodiscard]] constexpr bool valid() const noexcept {
            return max.x >= min.x && max.y >= min.y;
        }
    };
}
