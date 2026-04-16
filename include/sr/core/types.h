#pragma once

#include <cstdint>
#include <cstdlib>
#include <utility>
#include <expected>
#include <print>
#include <concepts>

#include "sr/core/error.h"

namespace sr {
    using u8 = std::uint8_t;
    using u16 = std::uint16_t;
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;

    using i8 = std::int8_t;
    using i16 = std::int16_t;
    using i32 = std::int32_t;
    using i64 = std::int64_t;

    using f32 = float;
    using f64 = double;

    static_assert(sizeof(f32) == 4);
    static_assert(sizeof(f64) == 8);

    using usize = std::size_t;
    using isize = std::ptrdiff_t;

    template<typename T>
    using Result = std::expected<T, Error>;

    using Status = Result<void>;

    template<typename T>
    constexpr auto unwrap(Result<T> res, const char *msg = "fatal error") -> T {
        if (!res) {
            std::println(stderr, "{}: {}", msg, to_string(res.error()));
            std::abort();
        }
        return std::move(*res);
    }

    template<typename T>
    concept Integral = std::integral<T> && !std::same_as<T, bool>;

    template<typename T>
    concept Floating = std::floating_point<T>;

    template<typename T>
    concept Number = (Integral<T> || Floating<T>) && !std::same_as<T, bool>;
}
