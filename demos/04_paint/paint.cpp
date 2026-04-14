#include "sr/sr.h"

#include <array>
#include <print>

constexpr sr::i32 WIDTH = 1920;
constexpr sr::i32 HEIGHT = 1080;
constexpr auto WIN_TITLE = "SR Paint";
constexpr sr::u32 TARGET_FPS = 60;

constexpr sr::Color BG{30, 30, 35};

constexpr std::array<sr::Color, 6> PALETTE = {
    sr::Color{230, 40, 40},
    sr::Color{40, 200, 80},
    sr::Color{60, 120, 230},
    sr::Color{240, 210, 60},
    sr::Color{230, 230, 230},
    sr::Color{20, 20, 25},
};

constexpr sr::f32 MIN_THICKNESS = 1.f;
constexpr sr::f32 MAX_THICKNESS = 40.f;

struct PaintState {
    sr::Color brush;
    sr::f32 thickness{};
    sr::Vec2f prev_mouse;
    bool was_drawing{};
};

static auto to_vec2i(sr::Vec2f v) noexcept -> sr::Vec2i {
    return {static_cast<sr::i32>(v.x()), static_cast<sr::i32>(v.y())};
}

// stamp filled circles along path from `from` to `to` with spacing ≤ radius
static auto stamp_stroke(sr::Renderer2D &ren, sr::Vec2f from, sr::Vec2f to, sr::i32 r, sr::Color c) noexcept -> void {
    const sr::Vec2f delta = to - from;
    const sr::f32 dist = delta.len();
    const auto step = static_cast<sr::f32>(r);
    const sr::i32 n = std::max(1, static_cast<sr::i32>(dist / step));
    for (sr::i32 i = 1; i <= n; ++i) {
        const sr::Vec2f p = from + delta * (static_cast<sr::f32>(i) / static_cast<sr::f32>(n));
        ren.fill_circle(to_vec2i(p), r, c);
    }
}

static auto handle_paint(sr::Window &win, sr::Renderer2D &ren, PaintState &s) noexcept -> void {
    const sr::Vec2f cur = win.mouse_pos();
    const bool lmb = win.mouse_down(sr::MouseButton::Left);
    const bool rmb = win.mouse_down(sr::MouseButton::Right);

    if (lmb || rmb) {
        const sr::Color c = rmb ? BG : s.brush;
        const sr::i32 r = std::max(1, static_cast<sr::i32>(s.thickness * 0.5f));
        const sr::Vec2f from = s.was_drawing ? s.prev_mouse : cur;
        stamp_stroke(ren, from, cur, r, c);
    }

    s.prev_mouse = cur;
    s.was_drawing = lmb || rmb;
}

static auto handle_controls(sr::Window &win, sr::Renderer2D &ren, PaintState &s) noexcept -> void {
    for (sr::usize i = 0; i < PALETTE.size(); ++i) {
        const auto key = static_cast<sr::Key>(static_cast<sr::i32>(sr::Key::Num1) + static_cast<sr::i32>(i));
        if (win.key_pressed(key)) {
            s.brush = PALETTE[i];
        }
    }

    if (win.key_pressed(sr::Key::LeftBracket)) {
        s.thickness = std::max(MIN_THICKNESS, s.thickness - 1.f);
    }
    if (win.key_pressed(sr::Key::RightBracket)) {
        s.thickness = std::min(MAX_THICKNESS, s.thickness + 1.f);
    }

    if (win.key_pressed(sr::Key::C)) {
        ren.clear(BG);
    }
}

static auto print_controls() noexcept -> void {
    std::println("SR Paint — controls:");
    std::println("  LMB     — paint");
    std::println("  RMB     — erase");
    std::println("  1-6     — color (red, green, blue, yellow, white, dark)");
    std::println("  [ / ]   — thinner / thicker brush");
    std::println("  C       — clear canvas");
}

int main() {
    auto win = sr::unwrap(sr::Window::create(WIN_TITLE, WIDTH, HEIGHT));
    auto fb = sr::unwrap(sr::FrameBuffer::create(WIDTH, HEIGHT));
    sr::Renderer2D ren{fb};

    sr::Window::set_target_fps(TARGET_FPS);

    print_controls();

    ren.clear(BG);

    PaintState state{
        .brush = PALETTE[0],
        .thickness = 4.f,
        .prev_mouse = win.mouse_pos(),
        .was_drawing = false,
    };

    while (win.is_open()) {
        handle_paint(win, ren, state);
        handle_controls(win, ren, state);
        win.present(fb);
    }
}
