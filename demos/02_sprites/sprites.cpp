#include "sr/sr.h"

constexpr sr::i32 WIDTH = 1920;
constexpr sr::i32 HEIGHT = 1080;
constexpr auto WIN_TITLE = "SR Sprites";
constexpr sr::u32 TARGET_FPS = 60;

constexpr auto TEX_NAME = "img.png";

int main() {
    auto win = sr::unwrap(sr::Window::create(WIN_TITLE, WIDTH, HEIGHT));
    auto fb = sr::unwrap(sr::FrameBuffer::create(WIDTH, HEIGHT));
    sr::Renderer2D ren{fb};

    const auto tex = sr::unwrap(sr::Texture::load(TEX_NAME), "failed to load texture");

    sr::Window::set_target_fps(TARGET_FPS);

    sr::f32 angle = 0.f;

    while (win.is_open()) {
        ren.clear({30, 30, 30});

        angle += win.dt();

        // simple blit
        ren.draw_sprite(tex, {100, 100});

        // scaled up
        ren.draw_sprite_ex(tex, {600, 100}, {0, 0}, {2.f, 2.f}, 0.f);

        // rotated
        const auto cx = static_cast<sr::f32>(tex.width()) / 2.f;
        const auto cy = static_cast<sr::f32>(tex.height()) / 2.f;
        ren.draw_sprite_ex(tex, {1200, 300}, {cx, cy}, {1.f, 1.f}, angle);

        // scaled + rotated
        ren.draw_sprite_ex(tex, {1600, 300}, {cx, cy}, {1.5f, 1.5f}, -angle * 0.5f);

        // mirrored horizontally
        ren.draw_sprite_ex(tex, {100, 600}, {0, 0}, {-1.f, 1.f}, 0.f);

        // small
        ren.draw_sprite_ex(tex, {500, 600}, {0, 0}, {0.5f, 0.5f}, 0.f);

        win.present(fb);
    }

    return 0;
}
