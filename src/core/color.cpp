#include "sr/core/color.h"

namespace sr {
    auto Color::blend_over(Color dst) const noexcept -> Color {
        const f32 sa = static_cast<f32>(a) / 255.f;
        const f32 inv = 1.f - sa;
        return {
            static_cast<u8>(static_cast<f32>(r) * sa + static_cast<f32>(dst.r) * inv + 0.5f),
            static_cast<u8>(static_cast<f32>(g) * sa + static_cast<f32>(dst.g) * inv + 0.5f),
            static_cast<u8>(static_cast<f32>(b) * sa + static_cast<f32>(dst.b) * inv + 0.5f),
            static_cast<u8>(static_cast<f32>(a) + static_cast<f32>(dst.a) * inv + 0.5f)
        };
    }

    auto Color::lerp(Color a, Color b, f32 t) noexcept -> Color {
        const f32 inv = 1.f - t;
        return {
            static_cast<u8>(static_cast<f32>(a.r) * inv + static_cast<f32>(b.r) * t + 0.5f),
            static_cast<u8>(static_cast<f32>(a.g) * inv + static_cast<f32>(b.g) * t + 0.5f),
            static_cast<u8>(static_cast<f32>(a.b) * inv + static_cast<f32>(b.b) * t + 0.5f),
            static_cast<u8>(static_cast<f32>(a.a) * inv + static_cast<f32>(b.a) * t + 0.5f)
        };
    }
}
