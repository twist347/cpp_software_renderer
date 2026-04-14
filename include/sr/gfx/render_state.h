#pragma once

#include "sr/core/types.h"

namespace sr {
    enum class BlendMode : u8 {
        None, // src overwrites dst (alpha ignored)
        Alpha, // Porter-Duff source-over: out = src.rgb*a + dst.rgb*(1-a)
        Additive, // out = saturate(dst + src.rgb*a)
    };

    enum class SamplerFilter : u8 {
        Nearest, // point sampling
        Bilinear, // 2x2 linear interpolation
    };

    enum class WrapMode : u8 {
        Clamp, // sample nearest edge for out-of-bounds uvs
        Repeat, // tile: uv mod size
        Mirror, // reflect: every other tile flipped
    };
}
