#pragma once

#include <algorithm>
#include <cassert>

#include "sr/core/vec.h"

namespace sr {
    // 2D rectangle with half-open intervals:
    // a point p is inside iff min.x <= p.x < max.x && min.y <= p.y < max.y.
    template<Number T>
    struct Rect {
        Vec<T, 2> min{};
        Vec<T, 2> max{};

        constexpr Rect() noexcept = default;

        constexpr Rect(const Vec<T, 2> &min, const Vec<T, 2> &max) noexcept : min{min}, max{max} {
        }

        static constexpr auto from_pos_size(Vec<T, 2> pos, Vec<T, 2> size) noexcept -> Rect {
            assert(size.x() >= T{0} && size.y() >= T{0});
            return {pos, pos + size};
        }

        [[nodiscard]] constexpr auto size() const noexcept -> Vec<T, 2> { return max - min; }
        [[nodiscard]] constexpr auto width() const noexcept -> T { return max.x() - min.x(); }
        [[nodiscard]] constexpr auto height() const noexcept -> T { return max.y() - min.y(); }

        [[nodiscard]] constexpr auto contains(Vec<T, 2> p) const noexcept -> bool {
            return p.x() >= min.x() && p.x() < max.x() && p.y() >= min.y() && p.y() < max.y();
        }

        [[nodiscard]] constexpr auto intersects(const Rect &o) const noexcept -> bool {
            return min.x() < o.max.x() && max.x() > o.min.x() && min.y() < o.max.y() && max.y() > o.min.y();
        }

        [[nodiscard]] constexpr auto clipped(const Rect &bounds) const noexcept -> Rect {
            return {
                {std::max(min.x(), bounds.min.x()), std::max(min.y(), bounds.min.y())},
                {std::min(max.x(), bounds.max.x()), std::min(max.y(), bounds.max.y())}
            };
        }

        [[nodiscard]] constexpr auto empty() const noexcept -> bool {
            return max.x() <= min.x() || max.y() <= min.y();
        }

        constexpr auto operator==(const Rect &) const noexcept -> bool = default;
    };

    using Recti = Rect<i32>;
    using Rectf = Rect<f32>;
}
