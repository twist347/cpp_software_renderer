#pragma once

#include <cmath>
#include <concepts>
#include <array>

#include "sr/core/types.h"

namespace sr {
    template<typename T>
    concept Integral = std::integral<T> && !std::same_as<T, bool>;

    template<typename T>
    concept Floating = std::floating_point<T>;

    template<typename T>
    concept Number = (Integral<T> || Floating<T>) && !std::same_as<T, bool>;

    template<Number T, usize N>
        requires (N > 0)
    struct Vec {
    private:
        std::array<T, N> m_data{};

    public:
        constexpr Vec() noexcept = default;

        template<typename... Args>
            requires (sizeof...(Args) == N && (std::convertible_to<Args, T> && ...))
        constexpr Vec(Args... args) noexcept : m_data{static_cast<T>(args)...} {
        }

        // broadcast: Vec2f{3.f} → {3.f, 3.f}. disabled for N == 1 to avoid overlap with variadic.
        explicit constexpr Vec(T v) noexcept requires (N > 1) {
            m_data.fill(v);
        }

        constexpr auto x(this auto &&self) noexcept -> auto & requires(N > 0) {
            return self.m_data[0];
        }

        constexpr auto y(this auto &&self) noexcept -> auto & requires(N > 1) {
            return self.m_data[1];
        }

        constexpr auto z(this auto &&self) noexcept -> auto & requires(N > 2) {
            return self.m_data[2];
        }

        constexpr auto operator+=(const Vec &v) noexcept -> Vec & {
            for (usize i = 0; i < N; ++i) {
                m_data[i] += v.m_data[i];
            }
            return *this;
        }

        constexpr auto operator-=(const Vec &v) noexcept -> Vec & {
            for (usize i = 0; i < N; ++i) {
                m_data[i] -= v.m_data[i];
            }
            return *this;
        }

        constexpr auto operator*=(T s) noexcept -> Vec & {
            for (usize i = 0; i < N; ++i) {
                m_data[i] *= s;
            }
            return *this;
        }

        constexpr auto operator/=(T s) noexcept -> Vec & {
            for (usize i = 0; i < N; ++i) {
                m_data[i] /= s;
            }
            return *this;
        }

        friend constexpr auto operator+(Vec lhs, const Vec &rhs) noexcept -> Vec {
            lhs += rhs;
            return lhs;
        }

        friend constexpr auto operator-(Vec lhs, const Vec &rhs) noexcept -> Vec {
            lhs -= rhs;
            return lhs;
        }

        friend constexpr auto operator*(Vec lhs, T rhs) noexcept -> Vec {
            lhs *= rhs;
            return lhs;
        }

        friend constexpr auto operator*(T lhs, Vec rhs) noexcept -> Vec {
            rhs *= lhs;
            return rhs;
        }

        friend constexpr auto operator/(Vec lhs, T rhs) noexcept -> Vec {
            lhs /= rhs;
            return lhs;
        }

        constexpr auto operator-() const noexcept -> Vec {
            Vec res;
            for (usize i = 0; i < N; ++i) {
                res.m_data[i] = -m_data[i];
            }
            return res;
        }

        constexpr auto dot(const Vec &v) const noexcept -> T {
            T res{};
            for (usize i = 0; i < N; ++i) {
                res += m_data[i] * v.m_data[i];
            }
            return res;
        }

        constexpr auto len_sq() const noexcept -> T {
            return dot(*this);
        }

        constexpr auto len() const noexcept -> T requires Floating<T> {
            return std::sqrt(len_sq());
        }

        constexpr auto normalized() const noexcept -> Vec requires Floating<T> {
            const auto l = len();
            return l > T{0} ? *this / l : Vec{};
        }

        static constexpr auto lerp(const Vec &a, const Vec &b, T t) noexcept -> Vec {
            return a + (b - a) * t;
        }

        constexpr auto operator==(const Vec &) const noexcept -> bool = default;

        // for structure binding
        template<usize I>
        constexpr auto get(this auto &&self) noexcept -> auto & {
            return self.m_data[I];
        }
    };

    using Vec2i = Vec<i32, 2>;
    using Vec2f = Vec<f32, 2>;
    using Vec3f = Vec<f32, 3>;
}

// for structure bindings

template<sr::Number T, sr::usize N>
    requires (N > 0)
struct std::tuple_size<sr::Vec<T, N> > : std::integral_constant<std::size_t, N> {
};

template<std::size_t I, sr::Number T, sr::usize N>
    requires (N > 0)
struct std::tuple_element<I, sr::Vec<T, N> > {
    using type = T;
};
