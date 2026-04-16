#include <gtest/gtest.h>

#include <cmath>

#include "sr/core/vec.h"

using namespace sr;

TEST(Vec, DefaultZeroes) {
    Vec2i v{};
    EXPECT_EQ(v.x(), 0);
    EXPECT_EQ(v.y(), 0);
}

TEST(Vec, VariadicCtor) {
    Vec2i v{3, 4};
    EXPECT_EQ(v.x(), 3);
    EXPECT_EQ(v.y(), 4);
}

TEST(Vec, BroadcastCtorFillsAll) {
    Vec2f v{7.f};
    EXPECT_FLOAT_EQ(v.x(), 7.f);
    EXPECT_FLOAT_EQ(v.y(), 7.f);

    Vec3f w{2.f};
    EXPECT_FLOAT_EQ(w.x(), 2.f);
    EXPECT_FLOAT_EQ(w.y(), 2.f);
    EXPECT_FLOAT_EQ(w.z(), 2.f);
}

TEST(Vec, AccessorIsMutableLvalue) {
    Vec2i v{1, 2};
    v.x() = 10;
    v.y() = 20;
    EXPECT_EQ(v, Vec2i(10, 20));
}

TEST(Vec, Equality) {
    EXPECT_EQ(Vec2i(1, 2), Vec2i(1, 2));
    EXPECT_NE(Vec2i(1, 2), Vec2i(1, 3));
}

TEST(Vec, Add) {
    EXPECT_EQ(Vec2i(1, 2) + Vec2i(3, 4), Vec2i(4, 6));
}

TEST(Vec, Sub) {
    EXPECT_EQ(Vec2i(5, 7) - Vec2i(1, 2), Vec2i(4, 5));
}

TEST(Vec, MulByScalar) {
    EXPECT_EQ(Vec2i(2, 3) * 4, Vec2i(8, 12));
    EXPECT_EQ(4 * Vec2i(2, 3), Vec2i(8, 12));
}

TEST(Vec, DivByScalar) {
    EXPECT_EQ(Vec2i(8, 12) / 4, Vec2i(2, 3));
}

TEST(Vec, CompoundAssign) {
    Vec2i v{1, 2};
    v += Vec2i(10, 20);
    EXPECT_EQ(v, Vec2i(11, 22));
    v -= Vec2i(1, 2);
    EXPECT_EQ(v, Vec2i(10, 20));
    v *= 2;
    EXPECT_EQ(v, Vec2i(20, 40));
    v /= 2;
    EXPECT_EQ(v, Vec2i(10, 20));
}

TEST(Vec, UnaryNegate) {
    EXPECT_EQ(-Vec2i(3, -4), Vec2i(-3, 4));
}

TEST(Vec, Dot) {
    EXPECT_EQ(Vec2i(1, 2).dot({3, 4}), 1 * 3 + 2 * 4);
    EXPECT_EQ(Vec3f(1.f, 2.f, 3.f).dot({4.f, 5.f, 6.f}), 32.f);
}

TEST(Vec, LenSqAndLen) {
    Vec2f v{3.f, 4.f};
    EXPECT_FLOAT_EQ(v.len_sq(), 25.f);
    EXPECT_FLOAT_EQ(v.len(), 5.f);
}

TEST(Vec, NormalizedUnit) {
    Vec2f v{3.f, 4.f};
    const auto n = v.normalized();
    EXPECT_NEAR(n.x(), 0.6f, 1e-6f);
    EXPECT_NEAR(n.y(), 0.8f, 1e-6f);
    EXPECT_NEAR(n.len(), 1.f, 1e-6f);
}

TEST(Vec, NormalizedZeroIsZero) {
    Vec2f v{};
    const auto n = v.normalized();
    EXPECT_FLOAT_EQ(n.x(), 0.f);
    EXPECT_FLOAT_EQ(n.y(), 0.f);
}

TEST(Vec, Lerp) {
    const auto m = Vec2f::lerp({0.f, 0.f}, {10.f, 20.f}, 0.5f);
    EXPECT_FLOAT_EQ(m.x(), 5.f);
    EXPECT_FLOAT_EQ(m.y(), 10.f);
}

TEST(Vec, LerpEndpoints) {
    Vec2f a{1.f, 2.f}, b{3.f, 4.f};
    EXPECT_EQ(Vec2f::lerp(a, b, 0.f), a);
    EXPECT_EQ(Vec2f::lerp(a, b, 1.f), b);
}

TEST(Vec, StructuredBinding) {
    auto [x, y] = Vec2i(11, 22);
    EXPECT_EQ(x, 11);
    EXPECT_EQ(y, 22);
}

TEST(Vec, StructuredBindingByRefMutates) {
    Vec2i v{1, 2};
    auto &[x, y] = v;
    x = 100;
    y = 200;
    EXPECT_EQ(v, Vec2i(100, 200));
}
