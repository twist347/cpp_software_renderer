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

        constexpr Vec2 operator+(Vec2 v) const noexcept { return {x + v.x, y + v.y}; }
        constexpr Vec2 operator-(Vec2 v) const noexcept { return {x - v.x, y - v.y}; }
        constexpr Vec2 operator*(T s) const noexcept { return {x * s, y * s}; }
        constexpr Vec2 operator/(T s) const noexcept { return {x / s, y / s}; }
        constexpr Vec2 operator-() const noexcept { return {-x, -y}; }

        constexpr T dot(Vec2 v) const noexcept { return x * v.x + y * v.y; }
        constexpr T cross(Vec2 v) const noexcept { return x * v.y - y * v.x; }
        constexpr T length_sq() const noexcept { return dot(*this); }

        [[nodiscard]] T length() const noexcept requires Floating<T> {
            return std::sqrt(length_sq());
        }

        [[nodiscard]] Vec2 normalized() const noexcept requires Floating<T> {
            const auto l = length();
            return l > T(0) ? *this / l : Vec2{};
        }

        constexpr Vec2 &operator+=(Vec2 o) noexcept {
            x += o.x;
            y += o.y;
            return *this;
        }

        constexpr Vec2 &operator-=(Vec2 o) noexcept {
            x -= o.x;
            y -= o.y;
            return *this;
        }

        constexpr Vec2 &operator*=(T s) noexcept {
            x *= s;
            y *= s;
            return *this;
        }

        static constexpr Vec2 lerp(Vec2 a, Vec2 b, T t) noexcept {
            return a + (b - a) * t;
        }

        constexpr bool operator==(const Vec2 &) const noexcept = default;
    };

    using Vec2i = Vec2<i32>;
    using Vec2f = Vec2<f32>;
    using Vec2d = Vec2<f64>;
}
