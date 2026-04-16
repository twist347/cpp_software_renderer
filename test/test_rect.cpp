#include <gtest/gtest.h>

#include "sr/core/aabb.h"
#include "sr/core/rect.h"

using namespace sr;

// ---------- Rect (half-open, integer pixel grid) ----------

TEST(Rect, CtorAndAccessors) {
    Recti r{{2, 3}, {10, 20}};
    EXPECT_EQ(r.min, Vec2i(2, 3));
    EXPECT_EQ(r.max, Vec2i(10, 20));
    EXPECT_EQ(r.width(), 8);
    EXPECT_EQ(r.height(), 17);
    EXPECT_EQ(r.size(), Vec2i(8, 17));
}

TEST(Rect, FromPosSize) {
    const auto r = Recti::from_pos_size({5, 5}, {10, 20});
    EXPECT_EQ(r.min, Vec2i(5, 5));
    EXPECT_EQ(r.max, Vec2i(15, 25));
}

TEST(Rect, ContainsHalfOpen) {
    const Recti r{{0, 0}, {10, 10}};
    EXPECT_TRUE(r.contains({0, 0})); // min included
    EXPECT_TRUE(r.contains({9, 9})); // last interior pixel
    EXPECT_FALSE(r.contains({10, 10})); // max excluded
    EXPECT_FALSE(r.contains({10, 0})); // x = max excluded
    EXPECT_FALSE(r.contains({0, 10})); // y = max excluded
    EXPECT_FALSE(r.contains({-1, 5}));
}

TEST(Rect, IntersectsTouchingEdgesIsFalse) {
    // half-open: rects sharing an edge do NOT overlap (no shared pixel)
    const Recti a{{0, 0}, {10, 10}};
    const Recti b{{10, 0}, {20, 10}};
    EXPECT_FALSE(a.intersects(b));
    EXPECT_FALSE(b.intersects(a));
}

TEST(Rect, IntersectsOverlap) {
    const Recti a{{0, 0}, {10, 10}};
    const Recti b{{5, 5}, {15, 15}};
    EXPECT_TRUE(a.intersects(b));
    EXPECT_TRUE(b.intersects(a));
}

TEST(Rect, IntersectsDisjoint) {
    const Recti a{{0, 0}, {10, 10}};
    const Recti b{{20, 20}, {30, 30}};
    EXPECT_FALSE(a.intersects(b));
}

TEST(Rect, ClippedFullyInside) {
    const Recti r{{2, 2}, {8, 8}};
    const Recti bounds{{0, 0}, {10, 10}};
    EXPECT_EQ(r.clipped(bounds), r);
}

TEST(Rect, ClippedPartial) {
    const Recti r{{-5, -5}, {5, 5}};
    const Recti bounds{{0, 0}, {10, 10}};
    EXPECT_EQ(r.clipped(bounds), Recti({0, 0}, {5, 5}));
}

TEST(Rect, ClippedNoOverlapProducesEmpty) {
    const Recti r{{20, 20}, {30, 30}};
    const Recti bounds{{0, 0}, {10, 10}};
    EXPECT_TRUE(r.clipped(bounds).empty());
}

TEST(Rect, EmptyConditions) {
    EXPECT_TRUE((Recti{{0, 0}, {0, 0}}).empty()); // zero size
    EXPECT_TRUE((Recti{{5, 5}, {5, 10}}).empty()); // zero width
    EXPECT_TRUE((Recti{{5, 5}, {10, 5}}).empty()); // zero height
    EXPECT_TRUE((Recti{{10, 10}, {5, 5}}).empty()); // inverted
    EXPECT_FALSE((Recti{{0, 0}, {1, 1}}).empty()); // single pixel
}

// ---------- AABB2D (closed, continuous floats) ----------

TEST(AABB2D, CtorAndAccessors) {
    AABB2D b{{0.f, 0.f}, {10.f, 20.f}};
    EXPECT_EQ(b.min, Vec2f(0.f, 0.f));
    EXPECT_EQ(b.max, Vec2f(10.f, 20.f));
    EXPECT_EQ(b.size(), Vec2f(10.f, 20.f));
    EXPECT_EQ(b.center(), Vec2f(5.f, 10.f));
}

TEST(AABB2D, FromPosSize) {
    const auto b = AABB2D::from_pos_size({1.f, 2.f}, {3.f, 4.f});
    EXPECT_EQ(b.min, Vec2f(1.f, 2.f));
    EXPECT_EQ(b.max, Vec2f(4.f, 6.f));
}

TEST(AABB2D, ContainsClosed) {
    const AABB2D b{{0.f, 0.f}, {10.f, 10.f}};
    EXPECT_TRUE(b.contains({0.f, 0.f})); // min included
    EXPECT_TRUE(b.contains({10.f, 10.f})); // max INCLUDED (closed)
    EXPECT_TRUE(b.contains({5.f, 5.f}));
    EXPECT_FALSE(b.contains({-0.1f, 5.f}));
    EXPECT_FALSE(b.contains({10.1f, 5.f}));
}

TEST(AABB2D, IntersectsTouchingEdgesIsTrue) {
    // closed: shared edge counts as intersection
    const AABB2D a{{0.f, 0.f}, {10.f, 10.f}};
    const AABB2D b{{10.f, 0.f}, {20.f, 10.f}};
    EXPECT_TRUE(a.intersects(b));
    EXPECT_TRUE(b.intersects(a));
}

TEST(AABB2D, IntersectsDisjoint) {
    const AABB2D a{{0.f, 0.f}, {10.f, 10.f}};
    const AABB2D b{{20.f, 20.f}, {30.f, 30.f}};
    EXPECT_FALSE(a.intersects(b));
}

TEST(AABB2D, ClippedPartial) {
    const AABB2D b{{-5.f, -5.f}, {5.f, 5.f}};
    const AABB2D bounds{{0.f, 0.f}, {10.f, 10.f}};
    const auto out = b.clipped(bounds);
    EXPECT_EQ(out.min, Vec2f(0.f, 0.f));
    EXPECT_EQ(out.max, Vec2f(5.f, 5.f));
}

TEST(AABB2D, EmptyConditions) {
    // closed semantics: zero-size box contains exactly one point → non-empty
    EXPECT_FALSE((AABB2D{{0.f, 0.f}, {0.f, 0.f}}).empty());
    EXPECT_TRUE((AABB2D{{1.f, 0.f}, {0.f, 1.f}}).empty()); // x inverted
    EXPECT_TRUE((AABB2D{{0.f, 1.f}, {1.f, 0.f}}).empty()); // y inverted
}
