#include "sr/core/color.h"
#include "sr/gfx/framebuffer.h"
#include "sr/gfx/rasterizer.h"
#include "sr/platform/window.h"

constexpr sr::i32 WIDTH = 800;
constexpr sr::i32 HEIGHT = 600;
constexpr sr::u32 TARGET_FPS = 60;

int main() {
    auto fb = sr::unwrap(sr::FrameBuffer::create(WIDTH, HEIGHT));
    auto win = sr::unwrap(sr::Window::create("SR Demo", WIDTH, HEIGHT));

    sr::Window::set_target_fps(TARGET_FPS);

    while (win.is_open()) {
        fb.clear(sr::colors::black);

        sr::raster::fill_rect(fb, {50, 50}, {250, 200}, sr::colors::red);
        sr::raster::draw_rect(fb, {300, 50}, {500, 200}, sr::colors::green);
        sr::raster::fill_circle(fb, {650, 125}, 75, sr::colors::blue);
        sr::raster::draw_circle(fb, {650, 400}, 100, sr::colors::white);
        sr::raster::line(fb, {50, 300}, {750, 550}, sr::Color{255, 255, 0});
        sr::raster::fill_triangle(fb, {400, 300}, {300, 500}, {500, 500},
                                  sr::Color{255, 0, 255});
        sr::raster::put_pixel(fb, 400, 300, sr::colors::white);

        win.present(fb);
    }

    return 0;
}
