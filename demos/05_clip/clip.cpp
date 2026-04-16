#include "sr/sr.h"

#include <cmath>

constexpr sr::i32 WIDTH = 1920;
constexpr sr::i32 HEIGHT = 1080;
constexpr auto WIN_TITLE = "SR Clip";
constexpr sr::u32 TARGET_FPS = 60;

int main() {
    auto win = sr::unwrap(sr::Window::create(WIN_TITLE, WIDTH, HEIGHT));
    auto fb = sr::unwrap(sr::FrameBuffer::create(WIDTH, HEIGHT));
    sr::Renderer2D ren{fb};

    sr::Window::set_target_fps(TARGET_FPS);

    sr::f32 t = 0.f;
    while (win.is_open()) {
        ren.clear(sr::Color{20, 20, 30});

        // unclipped background: dashed grid
        for (sr::i32 y = 0; y < HEIGHT; y += 60) {
            ren.draw_line({0, y}, {WIDTH, y}, sr::Color{60, 60, 80});
        }
        for (sr::i32 x = 0; x < WIDTH; x += 60) {
            ren.draw_line({x, 0}, {x, HEIGHT}, sr::Color{60, 60, 80});
        }

        // scissor window slides horizontally
        constexpr sr::i32 SCISSOR_W = 760;
        constexpr sr::i32 SCISSOR_H = 600;
        const auto cx = static_cast<sr::i32>(WIDTH * 0.5f + std::sin(t) * 460.f);
        const sr::Recti scissor = sr::Recti::from_pos_size(
            {cx - SCISSOR_W / 2, (HEIGHT - SCISSOR_H) / 2}, {SCISSOR_W, SCISSOR_H}
        );
        ren.set_clip(scissor);

        // everything below is clipped to the scissor rect
        ren.fill_rect({150, 150}, {WIDTH - 150, HEIGHT - 150}, sr::Color{200, 80, 80});
        ren.fill_circle({WIDTH / 2, HEIGHT / 2}, 260, sr::Color{80, 180, 220});
        ren.draw_line({0, 0}, {WIDTH, HEIGHT}, sr::colors::yellow);
        ren.draw_line({WIDTH, 0}, {0, HEIGHT}, sr::colors::yellow);

        ren.reset_clip();

        // frame around scissor rect (unclipped)
        ren.draw_rect(scissor.min, scissor.max - sr::Vec2i{1, 1}, sr::colors::white);

        win.present(fb);
        t += 0.02f;
    }
}
