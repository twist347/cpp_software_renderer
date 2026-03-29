#pragma once

#include <cmath>
#include <concepts>

#include "sr/core/types.h"

namespace sr {
    template<typename T>
    concept Integral = std::integral<T> && !std::same_as<T, bool>;

    template<typename T>
    concept Floating = std::floating_point<T>;

    template<typename T>
    concept Number = (Integral<T> || Floating<T>) && !std::same_as<T, bool>;

    template<Number T = f32>
    struct Vec2 {
        T x{};
        T y{};

        constexpr auto operator+(Vec2 v) const noexcept -> Vec2 { return {x + v.x, y + v.y}; }
        constexpr auto operator-(Vec2 v) const noexcept -> Vec2 { return {x - v.x, y - v.y}; }
        constexpr auto operator*(T s) const noexcept -> Vec2 { return {x * s, y * s}; }
        constexpr auto operator/(T s) const noexcept -> Vec2 { return {x / s, y / s}; }
        constexpr auto operator-() const noexcept -> Vec2 { return {-x, -y}; }

        constexpr auto dot(Vec2 v) const noexcept -> T { return x * v.x + y * v.y; }
        constexpr auto cross(Vec2 v) const noexcept -> T { return x * v.y - y * v.x; }
        constexpr auto length_sq() const noexcept -> T { return dot(*this); }

        [[nodiscard]] auto length() const noexcept -> T requires Floating<T> {
            return std::sqrt(length_sq());
        }

        [[nodiscard]] auto normalized() const noexcept -> Vec2 requires Floating<T> {
            const auto l = length();
            return l > T(0) ? *this / l : Vec2{};
        }

        constexpr auto operator+=(Vec2 o) noexcept -> Vec2 & {
            x += o.x;
            y += o.y;
            return *this;
        }

        constexpr auto operator-=(Vec2 o) noexcept -> Vec2 & {
            x -= o.x;
            y -= o.y;
            return *this;
        }

        constexpr auto operator*=(T s) noexcept -> Vec2 & {
            x *= s;
            y *= s;
            return *this;
        }

        static constexpr auto lerp(Vec2 a, Vec2 b, T t) noexcept -> Vec2 {
            return a + (b - a) * t;
        }

        constexpr bool operator==(const Vec2 &) const noexcept = default;
    };

    using Vec2i = Vec2<i32>;
    using Vec2f = Vec2<f32>;
    using Vec2d = Vec2<f64>;
}
