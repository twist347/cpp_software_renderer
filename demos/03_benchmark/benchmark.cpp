#include "sr/sr.h"

#include <array>
#include <cmath>
#include <print>
#include <span>
#include <vector>

constexpr sr::i32 WIDTH = 1920;
constexpr sr::i32 HEIGHT = 1080;
constexpr auto WIN_TITLE = "SR Benchmark";

constexpr sr::i32 TEX_SIZE = 64;

constexpr sr::usize N_RECTS = 2000;
constexpr sr::usize N_CIRCLES = 500;
constexpr sr::usize N_LINES = 3000;
constexpr sr::usize N_SPRITES = 100;

constexpr sr::u32 SEED = 42;

// -------- xorshift32 (must match Rust port bit-for-bit) --------
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

// -------- scene objects --------
struct MovingRect {
    sr::Vec2f pos;
    sr::Vec2f vel;
    sr::Vec2f size;
    sr::Color color;
};

struct MovingCircle {
    sr::Vec2f pos;
    sr::Vec2f vel;
    sr::i32 radius{};
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
    sr::f32 angle{};
    sr::f32 angular_vel{};
    sr::f32 scale{};
    sr::usize tex_idx{};
};

struct Scene {
    std::vector<MovingRect> rects;
    std::vector<MovingCircle> circles;
    std::vector<MovingLine> lines;
    std::vector<MovingSprite> sprites;
};

// -------- procedural textures --------

// radial gradient: opaque warm center → transparent edge
static auto make_radial_texture(sr::i32 size) noexcept -> sr::Texture {
    std::vector<sr::Pixel> buf(static_cast<sr::usize>(size) * size);
    const sr::f32 c = (size - 1) / 2.f;
    for (sr::i32 y = 0; y < size; ++y) {
        for (sr::i32 x = 0; x < size; ++x) {
            const sr::f32 dx = x - c;
            const sr::f32 dy = y - c;
            const sr::f32 d = std::sqrt(dx * dx + dy * dy) / c;
            const sr::u8 a = d >= 1.f ? 0 : static_cast<sr::u8>(255.f * (1.f - d));
            buf[static_cast<sr::usize>(y) * size + x] = sr::Color{255, 180, 80, a}.to_argb();
        }
    }
    return sr::unwrap(sr::Texture::from_pixels(size, size, buf));
}

// colored ring: alpha rises from 0 at d=0.5, peaks at d=0.75, falls to 0 at d=1
static auto make_ring_texture(sr::i32 size) noexcept -> sr::Texture {
    std::vector<sr::Pixel> buf(static_cast<sr::usize>(size) * size);
    const sr::f32 c = (size - 1) / 2.f;
    for (sr::i32 y = 0; y < size; ++y) {
        for (sr::i32 x = 0; x < size; ++x) {
            const sr::f32 dx = x - c;
            const sr::f32 dy = y - c;
            const sr::f32 d = std::sqrt(dx * dx + dy * dy) / c;
            sr::u8 a = 0;
            if (d >= 0.5f && d <= 1.f) {
                const sr::f32 t = 1.f - std::abs((d - 0.75f) * 4.f);
                a = static_cast<sr::u8>(255.f * t);
            }
            buf[static_cast<sr::usize>(y) * size + x] = sr::Color{80, 200, 255, a}.to_argb();
        }
    }
    return sr::unwrap(sr::Texture::from_pixels(size, size, buf));
}

// checkerboard: two alpha-varying colors, 8px tile — sharp transitions stress sampler
static auto make_checker_texture(sr::i32 size) noexcept -> sr::Texture {
    std::vector<sr::Pixel> buf(static_cast<sr::usize>(size) * size);
    for (sr::i32 y = 0; y < size; ++y) {
        for (sr::i32 x = 0; x < size; ++x) {
            constexpr sr::i32 TILE = 8;
            const bool on = ((x / TILE) + (y / TILE)) % 2 == 0;
            const sr::Color col = on
                                      ? sr::Color{220, 60, 140, 220}
                                      : sr::Color{80, 220, 180, 140};
            buf[static_cast<sr::usize>(y) * size + x] = col.to_argb();
        }
    }
    return sr::unwrap(sr::Texture::from_pixels(size, size, buf));
}

// white noise: per-pixel random rgb with fixed xorshift seed (deterministic)
static auto make_noise_texture(sr::i32 size) noexcept -> sr::Texture {
    std::vector<sr::Pixel> buf(static_cast<sr::usize>(size) * size);
    sr::u32 s = 0xCAFEBABEu;
    const auto next = [&] noexcept -> sr::u32 {
        s ^= s << 13;
        s ^= s >> 17;
        s ^= s << 5;
        return s;
    };
    for (sr::i32 y = 0; y < size; ++y) {
        for (sr::i32 x = 0; x < size; ++x) {
            const sr::u32 v = next();
            const auto r = static_cast<sr::u8>((v >> 16) & 0xFF);
            const auto g = static_cast<sr::u8>((v >> 8) & 0xFF);
            const auto b = static_cast<sr::u8>(v & 0xFF);
            const auto a = static_cast<sr::u8>(64 + ((v >> 24) & 0x7F));
            buf[static_cast<sr::usize>(y) * size + x] = sr::Color{r, g, b, a}.to_argb();
        }
    }
    return sr::unwrap(sr::Texture::from_pixels(size, size, buf));
}

// -------- helpers --------

static auto gen_color(Rng &rng) noexcept -> sr::Color {
    return sr::Color{
        static_cast<sr::u8>(rng.range_i32(0, 256)),
        static_cast<sr::u8>(rng.range_i32(0, 256)),
        static_cast<sr::u8>(rng.range_i32(0, 256)),
        static_cast<sr::u8>(rng.range_i32(64, 256)),
    };
}

static auto gen_vel(Rng &rng) noexcept -> sr::Vec2f {
    return {rng.range_f32(-200.f, 200.f), rng.range_f32(-200.f, 200.f)};
}

static auto bounce(sr::Vec2f &p, sr::Vec2f &v) noexcept -> void {
    constexpr auto fw = static_cast<sr::f32>(WIDTH);
    constexpr auto fh = static_cast<sr::f32>(HEIGHT);
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
}

static auto to_vec2i(sr::Vec2f v) noexcept -> sr::Vec2i {
    return {static_cast<sr::i32>(v.x()), static_cast<sr::i32>(v.y())};
}

// -------- scene lifecycle --------

static auto init_scene(Rng &rng, sr::usize n_textures) -> Scene {
    Scene s;

    s.rects.resize(N_RECTS);
    for (auto &[pos, vel, size, color]: s.rects) {
        pos = {rng.range_f32(0.f, WIDTH), rng.range_f32(0.f, HEIGHT)};
        vel = gen_vel(rng);
        size = {rng.range_f32(10.f, 80.f), rng.range_f32(10.f, 80.f)};
        color = gen_color(rng);
    }

    s.circles.resize(N_CIRCLES);
    for (auto &[pos, vel, radius, color]: s.circles) {
        pos = {rng.range_f32(0.f, WIDTH), rng.range_f32(0.f, HEIGHT)};
        vel = gen_vel(rng);
        radius = rng.range_i32(5, 40);
        color = gen_color(rng);
    }

    s.lines.resize(N_LINES);
    for (auto &[a, b, va, vb, color]: s.lines) {
        a = {rng.range_f32(0.f, WIDTH), rng.range_f32(0.f, HEIGHT)};
        b = {rng.range_f32(0.f, WIDTH), rng.range_f32(0.f, HEIGHT)};
        va = gen_vel(rng);
        vb = gen_vel(rng);
        color = gen_color(rng);
    }

    s.sprites.resize(N_SPRITES);
    for (auto &[pos, vel, angle, angular_vel, scale, tex_idx]: s.sprites) {
        pos = {rng.range_f32(0.f, WIDTH), rng.range_f32(0.f, HEIGHT)};
        vel = gen_vel(rng);
        angle = rng.range_f32(0.f, 6.2831855f);
        angular_vel = rng.range_f32(-2.f, 2.f);
        scale = rng.range_f32(0.2f, 0.6f);
        tex_idx = static_cast<sr::usize>(rng.range_i32(0, static_cast<sr::i32>(n_textures)));
    }

    return s;
}

static auto update_scene(Scene &s, sr::f32 dt) noexcept -> void {
    for (auto &r: s.rects) {
        r.pos += r.vel * dt;
        bounce(r.pos, r.vel);
    }
    for (auto &c: s.circles) {
        c.pos += c.vel * dt;
        bounce(c.pos, c.vel);
    }
    for (auto &l: s.lines) {
        l.a += l.va * dt;
        l.b += l.vb * dt;
        bounce(l.a, l.va);
        bounce(l.b, l.vb);
    }
    for (auto &sp: s.sprites) {
        sp.pos += sp.vel * dt;
        sp.angle += sp.angular_vel * dt;
        bounce(sp.pos, sp.vel);
    }
}

static auto render_scene(sr::Renderer2D &ren, const Scene &s, std::span<const sr::Texture> textures) noexcept -> void {
    constexpr sr::Vec2f tex_origin{TEX_SIZE / 2.f, TEX_SIZE / 2.f};

    for (const auto &sp: s.sprites) {
        ren.draw_sprite_ex(textures[sp.tex_idx], sp.pos, tex_origin, {sp.scale, sp.scale}, sp.angle);
    }

    for (const auto &r: s.rects) {
        ren.fill_rect(to_vec2i(r.pos), to_vec2i(r.pos + r.size), r.color);
    }

    for (const auto &c: s.circles) {
        ren.fill_circle(to_vec2i(c.pos), c.radius, c.color);
    }

    for (const auto &l: s.lines) {
        ren.draw_line(to_vec2i(l.a), to_vec2i(l.b), l.color);
    }
}

static auto print_scene_info() noexcept -> void {
    std::println("SR Benchmark — {}x{}", WIDTH, HEIGHT);
    std::println("  sprites: {}", N_SPRITES);
    std::println("  rects:   {}", N_RECTS);
    std::println("  circles: {}", N_CIRCLES);
    std::println("  lines:   {}", N_LINES);
}

int main() {
    auto win = sr::unwrap(sr::Window::create(WIN_TITLE, WIDTH, HEIGHT));
    auto fb = sr::unwrap(sr::FrameBuffer::create(WIDTH, HEIGHT));
    sr::Renderer2D ren{fb};

    const std::array<sr::Texture, 4> textures{
        make_radial_texture(TEX_SIZE),
        make_ring_texture(TEX_SIZE),
        make_checker_texture(TEX_SIZE),
        make_noise_texture(TEX_SIZE),
    };

    Rng rng{SEED};
    auto scene = init_scene(rng, textures.size());

    print_scene_info();

    while (win.is_open()) {
        update_scene(scene, win.dt());
        ren.clear({20, 20, 30});
        render_scene(ren, scene, textures);
        win.present(fb);
    }
}
