#include "sr/sr.h"

#include <print>
#include <vector>

constexpr sr::i32 WIDTH = 1280;
constexpr sr::i32 HEIGHT = 720;
constexpr auto WIN_TITLE = "SR Benchmark";
constexpr auto TEX_NAME = "img.png";

constexpr sr::usize N_RECTS = 2000;
constexpr sr::usize N_CIRCLES = 500;
constexpr sr::usize N_LINES = 3000;
constexpr sr::usize N_SPRITES = 100;

constexpr sr::u32 SEED = 42;

struct Rng {
    sr::u32 state;

    auto next_u32() noexcept -> sr::u32 {
        sr::u32 x = state;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        return state = x;
    }

    auto next_f32() noexcept -> sr::f32 {
        return static_cast<sr::f32>(next_u32()) / 4294967296.f;
    }

    auto range_i32(sr::i32 lo, sr::i32 hi) noexcept -> sr::i32 {
        return lo + static_cast<sr::i32>(next_f32() * static_cast<sr::f32>(hi - lo));
    }

    auto range_f32(sr::f32 lo, sr::f32 hi) noexcept -> sr::f32 {
        return lo + next_f32() * (hi - lo);
    }
};

struct MovingRect {
    sr::Vec2f pos;
    sr::Vec2f vel;
    sr::Vec2f size;
    sr::Color color;
};

struct MovingCircle {
    sr::Vec2f pos;
    sr::Vec2f vel;
    sr::i32 radius;
    sr::Color color;
};

struct MovingLine {
    sr::Vec2f a;
    sr::Vec2f b;
    sr::Vec2f va;
    sr::Vec2f vb;
    sr::Color color;
};

struct MovingSprite {
    sr::Vec2f pos;
    sr::Vec2f vel;
    sr::f32 angle;
    sr::f32 angular_vel;
    sr::f32 scale;
};

int main() {
    auto win = sr::unwrap(sr::Window::create(WIN_TITLE, WIDTH, HEIGHT));
    auto fb = sr::unwrap(sr::FrameBuffer::create(WIDTH, HEIGHT));
    sr::Renderer2D ren{fb};

    const auto tex = sr::unwrap(sr::Texture::load(TEX_NAME), "failed to load texture");

    Rng rng{SEED};

    const auto gen_color = [&] noexcept -> sr::Color {
        return sr::Color{
            static_cast<sr::u8>(rng.range_i32(0, 256)),
            static_cast<sr::u8>(rng.range_i32(0, 256)),
            static_cast<sr::u8>(rng.range_i32(0, 256)),
            static_cast<sr::u8>(rng.range_i32(64, 256)),
        };
    };

    const auto gen_vel = [&] noexcept -> sr::Vec2f {
        return {rng.range_f32(-200.f, 200.f), rng.range_f32(-200.f, 200.f)};
    };

    std::vector<MovingRect> rects(N_RECTS);
    for (auto &[pos, vel, size, color]: rects) {
        pos = {rng.range_f32(0.f, WIDTH), rng.range_f32(0.f, HEIGHT)};
        vel = gen_vel();
        size = {rng.range_f32(10.f, 80.f), rng.range_f32(10.f, 80.f)};
        color = gen_color();
    }

    std::vector<MovingCircle> circles(N_CIRCLES);
    for (auto &[pos, vel, radius, color]: circles) {
        pos = {rng.range_f32(0.f, WIDTH), rng.range_f32(0.f, HEIGHT)};
        vel = gen_vel();
        radius = rng.range_i32(5, 40);
        color = gen_color();
    }

    std::vector<MovingLine> lines(N_LINES);
    for (auto &[a, b, va, vb, color]: lines) {
        a = {rng.range_f32(0.f, WIDTH), rng.range_f32(0.f, HEIGHT)};
        b = {rng.range_f32(0.f, WIDTH), rng.range_f32(0.f, HEIGHT)};
        va = gen_vel();
        vb = gen_vel();
        color = gen_color();
    }

    std::vector<MovingSprite> sprites(N_SPRITES);
    for (auto &[pos, vel, angle, angular_vel, scale]: sprites) {
        pos = {rng.range_f32(0.f, WIDTH), rng.range_f32(0.f, HEIGHT)};
        vel = gen_vel();
        angle = rng.range_f32(0.f, 6.2831855f);
        angular_vel = rng.range_f32(-2.f, 2.f);
        scale = rng.range_f32(0.2f, 0.6f);
    }

    std::println("SR Benchmark — {}x{}", WIDTH, HEIGHT);
    std::println("  sprites: {}", N_SPRITES);
    std::println("  rects:   {}", N_RECTS);
    std::println("  circles: {}", N_CIRCLES);
    std::println("  lines:   {}", N_LINES);

    const auto fw = static_cast<sr::f32>(WIDTH);
    const auto fh = static_cast<sr::f32>(HEIGHT);

    const auto bounce = [fw, fh](sr::Vec2f &p, sr::Vec2f &v) noexcept {
        if (p.x() < 0.f) {
            p.x() = 0.f;
            v.x() = -v.x();
        }
        if (p.x() > fw) {
            p.x() = fw;
            v.x() = -v.x();
        }
        if (p.y() < 0.f) {
            p.y() = 0.f;
            v.y() = -v.y();
        }
        if (p.y() > fh) {
            p.y() = fh;
            v.y() = -v.y();
        }
    };

    const auto to_vec2i = [](sr::Vec2f v) noexcept -> sr::Vec2i {
        return {static_cast<sr::i32>(v.x()), static_cast<sr::i32>(v.y())};
    };

    const sr::Vec2f tex_origin{
        static_cast<sr::f32>(tex.width()) / 2.f,
        static_cast<sr::f32>(tex.height()) / 2.f,
    };

    while (win.is_open()) {
        const sr::f32 dt = win.dt();

        for (auto &r: rects) {
            r.pos += r.vel * dt;
            bounce(r.pos, r.vel);
        }
        for (auto &c: circles) {
            c.pos += c.vel * dt;
            bounce(c.pos, c.vel);
        }
        for (auto &l: lines) {
            l.a += l.va * dt;
            l.b += l.vb * dt;
            bounce(l.a, l.va);
            bounce(l.b, l.vb);
        }
        for (auto &s: sprites) {
            s.pos += s.vel * dt;
            s.angle += s.angular_vel * dt;
            bounce(s.pos, s.vel);
        }

        ren.clear({20, 20, 30});

        for (const auto &s: sprites) {
            ren.draw_sprite_ex(tex, s.pos, tex_origin, {s.scale, s.scale}, s.angle);
        }

        for (const auto &r: rects) {
            const auto p = to_vec2i(r.pos);
            const auto q = to_vec2i(r.pos + r.size);
            ren.fill_rect(p, q, r.color);
        }

        for (const auto &c: circles) {
            ren.fill_circle(to_vec2i(c.pos), c.radius, c.color);
        }

        for (const auto &l: lines) {
            ren.draw_line(to_vec2i(l.a), to_vec2i(l.b), l.color);
        }

        win.present(fb);
    }
}
