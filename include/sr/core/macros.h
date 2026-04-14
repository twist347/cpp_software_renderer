#pragma once

#if defined(__GNUC__) || defined(__clang__)
    #define SR_INLINE   inline __attribute__((always_inline))
    #define SR_NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
    #define SR_INLINE   __forceinline
    #define SR_NOINLINE __declspec(noinline)
#else
    #define SR_INLINE inline
    #define SR_NOINLINE
#endif
